
#include "../inc/p1.h"
#include "../inc/main.h"
#include "../inc/picohttpparser.h"

int execute_script(){

}

void process_GET(int connfd, size_t path_len, char *source, char* method)
{
	char buf[MAX_STRING], method2[MAX_CHAR], py[MAX_STRING]= ".py", php[MAX_CHAR]= ".php";

	struct stat st;
	int f;
	//###############
	char *path, *value = NULL, *trash;
	printf("%s\n\n", source);
	if (strrchr(source, '?') != NULL)
	{
		path = strtok(source, "?");
		trash = strtok(NULL, strcat(method, "="));
		value = strtok(NULL, method);

		if (strstr(source, py) != NULL || strstr(source, php) != NULL)
		{
			/* Hacemos el ejecutar fichero y tal */
			
		}
	}
	//################

	if ((int)path_len > 1)
	{
		f = open(source, O_RDONLY);
		if (f == -1)
		{
			printf("%s FALLO\n", source);
		}
	}
	else
	{
		f = open("templates/index.html", O_RDONLY);
		if (f == -1)
		{
			printf("templates FALLO\n");
		}
	}

	fstat(f, &st);
	char buffer[MAX_STRING];
	char size[MAX_STRING];
	sprintf(buffer, "HTTP/1.1 200 OK\r\n");
	strcat(buffer, "Date: Tue, 08 Sep 2020 00:53:20 GMT\r\n");
	strcat(buffer, "Server: Apache/2.4.6 (CentOS)\r\n");
	strcat(buffer, "Last-Modified: Tue, 01 Mar 2016 18:57:50 GMT\r\n");
	strcat(buffer, "Accept-Ranges: bytes\r\n");
	printf(size, "Content-Length: %li\r\n", st.st_size);
	strcat(buffer, size);
	strcat(buffer, "Content-Type: text/html\r\n\r\n");
	write(connfd, buffer, strlen(buffer));
	sendfile(connfd, f, NULL, st.st_size);
	close(f);
}

void process_POST(int connfd, size_t path_len, char *source, char *method)
{
	write(connfd, "No se que hacer aqui xdd\r\n\r\n", strlen("No se que hacer aqui xdd\r\n\r\n"));
}
void process_OPTIONS(int connfd)
{
	// curl -X OPTIONS localhost:8080 -i
	char buffer[500];
	sprintf(buffer, "HTTP/1.1 204 No Content\r\n");
	strcat(buffer, "Allow: OPTIONS, GET, POST\r\n");
	strcat(buffer, "Date: Tue, 08 Sep 2020 00:53:20 GMT\r\n");
	strcat(buffer, "Server: Apache 2.4.6 (CentOS)\r\n");
	strcat(buffer, "Last-Modified: Tue, 01 Mar 2016 18:57:50 GMT\r\n");
	strcat(buffer, "Accept-Ranges: bytes\r\n");
	strcat(buffer, "Content-Length: 0\r\n");
	strcat(buffer, "Content-Type: text/html\r\n\r\n");
	write(connfd, buffer, strlen(buffer));
}

// Function designed for chat between client and server.
void processRequest(int connfd)
{
	char buf[4096], method2[10], source[MAX_STRING] = "templates";
	const char *method, *path;
	int pret, minor_version, f, size;
	struct phr_header headers[100];
	size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
	ssize_t rret;
	
	while (1)
	{
		/* read the request */
		while ((rret = read(connfd, buf + buflen, sizeof(buf) - buflen)) == -1 && errno == EINTR)
			;
		if (rret <= 0)
			return;
		prevbuflen = buflen;
		buflen += rret;
		/* parse the request */
		num_headers = sizeof(headers) / sizeof(headers[0]);
		pret = phr_parse_request(buf, buflen, &method, &method_len, &path, &path_len,
								 &minor_version, headers, &num_headers, prevbuflen);
		if (pret > 0)
			break; /* successfully parsed the request */
		else if (pret == -1)
			return; // ParseError;
		/* request is incomplete, continue the loop */
		assert(pret == -2);
		if (buflen == sizeof(buf))
			return; // Request incomplete
	}

	printf("request is %d bytes long\n", pret);
	printf("method is %.*s\n", (int)method_len, method);
	printf("path is %.*s\n", (int)path_len, path);
	printf("HTTP version is 1.%d\n", minor_version);
	printf("headers:\n");
	for (int i = 0; i != num_headers; ++i)
	{
		printf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
			   (int)headers[i].value_len, headers[i].value);
	}

	/*Store request method*/
	sprintf(method2, "%.*s", (int)method_len, method);
	strncat(source, path, (int)path_len);

	/*Calls pertinent function*/
	
	if (strcmp(method2, "GET") == 0)
	{
		process_GET(connfd, path_len, source, method2);
	}
	else if (strcmp(method2, "POST") == 0)
	{

		process_POST(connfd, path_len, source, method2);
	}
	else if (strcmp(method2, "OPTIONS") == 0) 
	{
		process_OPTIONS(connfd);
	}
	else
	{
		//process_unsupported();
	}
	
}


void *pthread_main(void *socketfd)
{
	int serverfd = *((int *)socketfd);
	int connfd;
	while(1){
		connfd = acceptClient(serverfd);
		processRequest(connfd);
		close(connfd);
	}
}

int main(){

	int *info;
	char line[MAX_STRING], route[MAX_CHAR], name[MAX_CHAR];
	char numCli[MAX_CHAR], numPort[MAX_CHAR];
	FILE *fp;
	fp = fopen("config.conf", "r");
	if (fp == NULL)
	{
		//syslog(LOG_ERR, "Error opening file server.conf: %d", errno);
		return -1;
	}
	
	//Configuracion del servidor
	while (fgets(line, MAX_STRING, fp))
	{
		if (strncmp("server_root", line, strlen("server_root")) == 0)
		{
			strtok(line, " \n");
			strtok(NULL, " \n");
			sprintf(route, "%s", strtok(NULL, " \n"));
		}
		if (strncmp("max_clients", line, strlen("max_clients")) == 0)
		{
			strtok(line, " \n");
			strtok(NULL, " \n");
			sprintf(numCli, "%s", strtok(NULL, " \n"));
		}
		if (strncmp("listen_port", line, strlen("listen_port")) == 0)
		{
			strtok(line, " \n");
			strtok(NULL, " \n");
			sprintf(numPort, "%s", strtok(NULL, " \n"));
		}
		if (strncmp("server_signature", line, strlen("server_signature")) == 0)
		{
			strtok(line, " \n");
			strtok(NULL, " \n");
			sprintf(name, "%s", strtok(NULL, " \n"));
		}
	}

	info = initserverSocket(LISTEN, atoi(numPort));
	if (!info)
		exit(-1);

	//Los hilos pa
	// #####################
	pthread_t *threads;
	threads = (pthread_t*) malloc(sizeof(pthread_t)* atoi(numCli));

	for (int i = 0; i < atoi(numCli); i++)
	{
		pthread_create(&threads[i], NULL,pthread_main, (void *) &info[0]);
	}
	for (int i = 0; i < atoi(numCli); i++)
	{
		pthread_join(threads[i], NULL);
	}
	// #####################

	free(threads);
	freeSocket(info);
}
