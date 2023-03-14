/**
 * @brief Implements the functionality associated with the server and HTTP requests.
 *
 * @file server.c
 * @author Alvaro Rodriguez & Pablo Sanchez
 * @version 1.0
 * @date 13/03/2023
 * @copyright GNU Public License
 */

#include "../inc/p1.h"
#include "../inc/main.h"
#include "../inc/picohttpparser.h"

pthread_t *threads;
char numCli[SMALL_STRING];
char name[SMALL_STRING];
char route[SMALL_STRING];

/** @struct extension_map
 *
 *  @brief Holds the extension associated to a specific file
 */
typedef struct
{
	const char *extension;
	const char *mime_type;
} extension_map;

extension_map extensions_types[] = {
	{".txt", "text/plain"},
	{".html", "text/html"},
	{".htm", "text/html"},
	{".gif", "image/gif"},
	{".jpeg", "image/jpeg"},
	{".jpg", "image/jpeg"},
	{".mpeg", "video/mpeg"},
	{".mpg", "video/mpeg"},
	{".doc", "application/msword"},
	{".docx", "application/msword"},
	{".pdf", "application/pdf"},
	{".py", "text/html"},
	{".php", "text/html"},
	{NULL, NULL},
};

/**
 * @brief its a hash that given the request it gives a certain number
 * its only used to make clear code
 *
 * @param str the type of request
 * @return int a number
 */
int hash(char *str)
{
	if (strcmp(str, GET) == 0)
		return 0;
	else if (strcmp(str, POST) == 0)
		return 1;
	else if (strcmp(str, OPTIONS) == 0)
		return 2;
	else
		return -1;
}
/**
 * @brief executes a script either python3 or php storing the output inside a file descriptor which is returned
 *
 * @param path the path to the script
 * @param arg the arguments of the script
 * @param extension python3 or php
 * @return int file descriptor to the pipe where the output is stored
 */
int execute_script(char *path, char *arg, int extension)
{
	char command[MEDIUM_STRING];
	pid_t pid;
	int child_out[PIPE_TAM];
	int child_in[PIPE_TAM];

	pipe(child_in);
	pipe(child_out);
	if (extension == PHP_NUM)
		sprintf(command, "php %s ", path);
	else if (extension == PY_NUM)
		sprintf(command, "python3 %s", path);

	pid = fork();
	if (pid > 0)
	{
		close(child_in[READ]);
		close(child_out[WRITE]);
		write(child_in[WRITE], arg, strlen(arg));
		close(child_in[WRITE]);
		wait(NULL);
	}
	else
	{
		close(child_out[READ]);
		close(child_in[WRITE]);
		dup2(child_in[READ], STDIN_FILENO);
		dup2(child_out[WRITE], STDOUT_FILENO);
		if (extension == 0)
		{
			if (execlp("php", "php", path, (char *)0) == -1)
			{
				syslog(LOG_USER, "Error executing php script");
				return -1;
			}
		}
		else if (execlp("python3", "python3", path, (char *)0) == -1)
		{

			syslog(LOG_USER, "Error executing python3 script");
			return -1;
		}

		exit(EXIT_SUCCESS);
	}
	return child_out[READ];
}

/**
 * @brief process a 404 request, which is the code for not found file
 *
 * @param connfd
 */
void process_404_NotFound(int connfd)
{
	write(connfd, "HTTP/1.1 404 Not Found\r\n\r\n", strlen("HTTP/1.1 404 Not Found\r\n\r\n"));
}

/**
 * @brief process a 400 request which is the code for an unvalid request
 *
 * @param connfd the socket of the client
 */
void process_400_BadRequest(int connfd)
{
	write(connfd, "HTTP/1.1 400 Bad Request\r\n\r\n", strlen("HTTP/1.1 400 Bad Request\r\n\r\n"));
}

/**
 * @brief Processes a GET request creating the response needed
 *
 * @param connfd  the socket of the client
 * @param path_len  the size of the path
 * @param source  extra info
 */
void process_GET(int connfd, size_t path_len, char *source)
{
	char method[SMALL_STRING] = GET;
	char buffer[BIG_STRING];
	char extra[BIG_STRING];
	char timevar[MEDIUM_STRING];
	char exten[SMALL_STRING];
	char *output;
	char *path;
	char *value;
	char *trash;
	char *dot;
	struct stat st, attr;
	int f;
	long int size_int = 0;
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	extension_map *map = extensions_types;

	strftime(timevar, sizeof(timevar), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	if (strrchr(source, '?') != NULL)
	{
		path = strtok(source, "?");
		value = strtok(NULL,"=");
		value = strtok(NULL,"=");

		if (strstr(path, PHP_EXTENSION) != NULL)
		{
			f = execute_script(path, value, PHP_NUM);
			if (f == -1)
			{
				syslog(LOG_USER, "The file requested does not exist\n");
				process_404_NotFound(connfd);
				return;
			}
		}
		else if (strstr(path, PY_EXTENSION) != NULL)
		{
			f = execute_script(path, value, PY_NUM);
			if (f == -1)
			{
				syslog(LOG_USER, "The file requested does not exist\n");
				process_404_NotFound(connfd);
				return;
			}
		}
		ioctl(f, FIONREAD, &size_int);
		stat(source, &attr);

		/*Stores the extension of the file*/
		dot = strrchr(source, '.');

		/*Map the extension with the corresponding mime-type*/
		while (map->extension)
		{
			if (strcmp(map->extension, dot) == 0)
			{
				sprintf(exten, "%s", map->mime_type);
				break;
			}
			map++;
		}

		output = (char *)malloc(sizeof(char) * (size_int + 1));
		sprintf(buffer, "HTTP/1.1 200 OK\r\n");
		sprintf(extra,"Date: %s\r\n",timevar);
		strcat(buffer,extra);
		sprintf(extra,"Server: %s\r\n", name);
		strcat(buffer,extra);
		sprintf(extra, "Last-Modified: %s GMT\r\n", ctime(&attr.st_mtime));
		strcat(buffer, extra);
		sprintf(extra, "Content-Length: %li\r\n", size_int);
		strcat(buffer, extra);
		sprintf(extra, "Content-Type: %s\r\n\r\n", exten);
		strcat(buffer, extra);
		write(connfd, buffer, strlen(buffer));
		read(f, output, size_int);
		output[size_int] = '\0';
		write(connfd, output, strlen(output));
		free(output);
		close(f);
	}
	else
	{
		if ((int)path_len > 1)
		{
			f = open(source, O_RDONLY);
			if (f == -1)
			{
				syslog(LOG_USER, "The file requested does not exist\n");
				process_404_NotFound(connfd);
				return;
			}
			/*Stores the extension of the file*/
			dot = strrchr(source, '.');
			while (map->extension)
			{
				if (strcmp(map->extension, dot) == 0)
				{
					sprintf(exten, "%s", map->mime_type);
					break;
				}
				map++;
			}
		}
		else
		{
			f = open("templates/index.html", O_RDONLY);
			if (f == -1)
			{
				syslog(LOG_USER, "The file requested does not exist\n");
				process_404_NotFound(connfd);
				return;
			}
			strcpy(exten, "text/html");
		}

		fstat(f, &st);
		stat(source, &attr);
		sprintf(buffer, "HTTP/1.1 200 OK\r\n");
		sprintf(extra,"Date: %s\r\n",timevar);
		strcat(buffer,extra);
		sprintf(extra,"Server: %s\r\n", name);
		strcat(buffer, extra);
		sprintf(extra, "Last-Modified: %s GMT\r\n", ctime(&attr.st_mtime));
		strcat(buffer, extra);
		sprintf(extra, "Content-Length: %li\r\n", st.st_size);
		strcat(buffer, extra);
		sprintf(extra, "Content-Type: %s\r\n\r\n", exten);
		strcat(buffer, extra);
		write(connfd, buffer, strlen(buffer));
		sendfile(connfd, f, NULL, st.st_size);
		close(f);
	}
}

/**
 * @brief Processes a post request creating the response needed
 *
 * @param connfd  the socket of the client
 * @param source it has the path of the file
 * @param buff Contains de body of the request to get the variables of it
 */
void process_POST(int connfd, char *source, char *buff)
{
	char *path;
	char *output;
	char *variables;
	char *dot;
	char buffer[BIG_STRING];
	char extra[BIG_STRING];
	char timevar[MEDIUM_STRING];
	char exten[SMALL_STRING];
	long int size_int = 0;
	struct stat attr;
	extension_map *map = extensions_types;

	int f;
	int i;
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(timevar, sizeof(timevar), "%a, %d %b %Y %H:%M:%S %Z", &tm);


	variables = strtok(buff, "=");
	variables = strtok(NULL, "=");

	if (strrchr(source, '?') != NULL)
	{
		path = strtok(source, "?");

		if (strstr(path, PHP_EXTENSION) != NULL)
		{
			f = execute_script(path, variables, PHP_NUM);
			if (f == -1)
			{
				syslog(LOG_USER, "The file requested does not exist\n");
				process_404_NotFound(connfd);
				return;
			}
		}
		if (strstr(path, PY_EXTENSION) != NULL)
		{
			f = execute_script(path, variables, PY_NUM);
			if (f == -1)
			{
				syslog(LOG_USER, "The file requested does not exist\n");
				process_404_NotFound(connfd);
				return;
			}
		}
	}
	else
	{
		if (strstr(source, PY_EXTENSION) != NULL)
		{
			f = execute_script(source, variables, 1);
			if (f == -1)
			{
				syslog(LOG_USER, "The file requested does not exist\n");
				process_404_NotFound(connfd);
				return;
			}
		}
		else if (strstr(source, PHP_EXTENSION) != NULL)
		{
			f = execute_script(source, variables, 0);
			if (f == -1)
			{
				syslog(LOG_USER, "The file requested does not exist\n");
				process_404_NotFound(connfd);
				return;
			}
		}
	}
	/*Stores the extension of the file*/
	dot = strrchr(source, '.');
	while (map->extension)
	{
		if (strcmp(map->extension, dot) == 0)
		{
			sprintf(exten, "%s", map->mime_type);
			break;
		}
		map++;
	}

	ioctl(f, FIONREAD, &size_int);
	stat(source, &attr);
	output = (char *)malloc(sizeof(char) * (size_int + 1));
	sprintf(buffer, "HTTP/1.1 200 OK\r\n");
	sprintf(extra, "Date: %s\r\n", timevar);
	strcat(buffer, extra);
	sprintf(extra,"Server: %s\r\n", name);
	strcat(buffer,extra);
	sprintf(extra, "Last-Modified: %s GMT\r\n", ctime(&attr.st_mtime));
	strcat(buffer, extra);
	sprintf(extra, "Content-Length: %li\r\n", size_int);
	strcat(buffer, extra);
	sprintf(extra, "Content-Type: %s\r\n\r\n", exten);
	strcat(buffer, extra);
	write(connfd, buffer, strlen(buffer));
	read(f, output, size_int);
	output[size_int] = '\0';
	write(connfd, output, strlen(output));
	free(output);
	close(f);
}

/**
 * @brief Processes a OPTIONS request, sending the options the server accepts
 *
 * @param connfd the open socketfd of the client
 */
void process_OPTIONS(int connfd)
{
	char buffer[BIG_STRING];
	char extra[BIG_STRING];
	char timevar[MEDIUM_STRING];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(timevar, sizeof(time), "%a, %d %b %Y %H:%M:%S %Z", &tm);

	strcat(buffer, "Accept-Ranges: bytes\r\n");

	sprintf(buffer, "HTTP/1.1 204 No Content\r\n");
	strcat(buffer, "Allow: OPTIONS, GET, POST\r\n");
	sprintf(extra, "Date: %s\r\n", timevar);
	strcat(buffer, extra);
	sprintf(extra,"Server: %s\r\n", name);
	strcat(buffer,extra);
	strcat(buffer, "Content-Length: 0\r\n");
	strcat(buffer, "Content-Type: text/html\r\n\r\n");
	write(connfd, buffer, strlen(buffer));
}

/**
 * @brief Procesa una petición de un cliente, a través de la función phr_parse_request obtenemos
 * la petición parseada y de ahí descomponemos esta petición para devolver lo que se nos solicita
 *
 * @param connfd el socket del cliente abierto
 */
void processRequest(int connfd)
{
	char buf[BIG_STRING];
	char method2[SMALL_STRING];
	char source[SMALL_STRING];
	const char *method;
	const char *path;
	int pret;
	int minor_version;
	int f;
	int size;
	struct phr_header headers[MEDIUM_ARR];
	size_t buflen;
	size_t prevbuflen;
	size_t method_len;
	size_t path_len;
	size_t num_headers;
	ssize_t rret;

	buflen = 0;
	prevbuflen = 0;
	strcpy(source,route);

	while (TRUE)
	{
		while ((rret = read(connfd, buf + buflen, sizeof(buf) - buflen)) == -1 && errno == EINTR)
			;
		if (rret <= 0)
			return;
		prevbuflen = buflen;
		buflen += rret;
		num_headers = sizeof(headers) / sizeof(headers[0]);
		pret = phr_parse_request(buf, buflen, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers, prevbuflen);
		if (pret > 0)
			break;
		else if (pret == -1)
			return;
		assert(pret == -2);
		if (buflen == sizeof(buf))
			return;
	}
	sprintf(method2, "%.*s", (int)method_len, method);
	strncat(source, path, (int)(path_len));
	buf[buflen] = '\0';
	switch (hash(method2))
	{
	case 0:
		process_GET(connfd, path_len, source);
		break;
	case 1:
		process_POST(connfd, source, buf + pret);
		break;
	case 2:
		process_OPTIONS(connfd);
		break;
	case -1:
		process_400_BadRequest(connfd);
		break;
	}
}

/**
 * @brief Es la función principal que ejecutarán los hilos, en ella se quedarán a la espera de la llegada de un cliente, una vez que
 * llegue el cliente
 *
 * @param socketfd descriptor de ficheros del socket
 * @return void* aunque devuelve void * verdaderamente no necesitamos que devuelva nada
 */
void *pthread_main(void *socketfd)
{
	int serverfd;
	int connfd;

	serverfd = *((int *)socketfd);
	while (1)
	{
		connfd = acceptClient(serverfd);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		if (connfd == BASIC_ERROR)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			continue;
		}
		processRequest(connfd);
		close(connfd);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
	return NULL;
}
/**
 * @brief handler of the signal
 *
 */
void handler()
{
	for (int i = 0; i < atoi(numCli); i++)
	{
		pthread_cancel(threads[i]);
	}
}

/**
 * @brief creates handlers of signals
 *
 * @return int
 */
int create_handlers(void)
{
	struct sigaction usr1;

	usr1.sa_sigaction = handler;
	sigemptyset(&(usr1.sa_mask));
	sigaddset(&(usr1.sa_mask), SIGINT);
	usr1.sa_flags = SA_SIGINFO;
	if (sigaction(SIGINT, &usr1, NULL) < 0)
	{
		syslog(LOG_USER, "Error executing php script");
		return (ERROR);
	}
	return (OK);
}

/**
 * @brief Punto de entrada del servidor, lee la información del fichero de configuración e inicializa los hilos que se encargarán
 * de manejar las peticiones de los clientes
 *
 * @return int codigo de estado según si ha salido correctamente o no
 */
int main()
{

	int *info;
	char line[SMALL_STRING];
	char numPort[SMALL_STRING];
	FILE *fp;
	fp = fopen("config.conf", "r");
	if (!fp)
	{
		syslog(LOG_USER, "Error openning conf file\n");
		return -1;
	}

	while (fgets(line, SMALL_STRING, fp))
	{
		if (strncmp("server_root", line, strlen("server_root")) == 0)
		{
			strtok(line, " \n");
			strtok(NULL, " \n");
			sprintf(route, "%s", strtok(NULL, " \n"));
		}
		else if (strncmp("max_clients", line, strlen("max_clients")) == 0)
		{
			strtok(line, " \n");
			strtok(NULL, " \n");
			sprintf(numCli, "%s", strtok(NULL, " \n"));
		}
		else if (strncmp("listen_port", line, strlen("listen_port")) == 0)
		{
			strtok(line, " \n");
			strtok(NULL, " \n");
			sprintf(numPort, "%s", strtok(NULL, " \n"));
		}
		else if (strncmp("server_signature", line, strlen("server_signature")) == 0)
		{
			strtok(line, " \n");
			strtok(NULL, " \n");
			sprintf(name, "%s", strtok(NULL, " \n"));
		}
	}

	info = initserverSocket(LISTEN, atoi(numPort));
	if (!info)
		exit(-1);

	threads = (pthread_t *)malloc(sizeof(pthread_t) * atoi(numCli));
	if (!threads)
	{
		syslog(LOG_USER, "Error allocating memory for the threads\n");
		exit(-1);
	}
	if (create_handlers() != ERROR)
	{
		for (int i = 0; i < atoi(numCli); i++)
			pthread_create(&threads[i], NULL, pthread_main, (void *)&info[0]);
				
		for (int i = 0; i < atoi(numCli); i++)
			pthread_join(threads[i], NULL);
		free(threads);
		fclose(fp);
		freeSocket(info);
		exit(EXIT_SUCCESS);
	}
	else
	{
		fclose(fp);
		free(threads);
		freeSocket(info);
		exit(EXIT_FAILURE);
	}
}