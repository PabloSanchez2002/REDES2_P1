
#include "../inc/p1.h"
#include "../inc/main.h"
#include "../inc/picohttpparser.h"

// Function designed for chat between client and server.
void func(int connfd)
{
	char buf[4096], method2[10];
	const char *method, *path;
	int pret, minor_version, f, size;
	struct phr_header headers[100];
	struct stat st;
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

	char *addr, source[] = "templates";

	if ((int)path_len > 1)
	{
		printf("\n\nPath:%.*s\n\n", (int)path_len, path);
		addr = (char *)malloc((int)path_len);
		memcpy(addr, path, (int)path_len);
		strcat(source, addr);
		f = open(source, O_RDONLY);
		if(f == -1){
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
	char buffer[500];
	sprintf(buffer, "HTTP/1.1 200 OK\r\n\
Date: Tue, 08 Sep 2020 00:53:20 GMT\r\n\
Server: Apache/2.4.6 (CentOS)\r\n\
Last-Modified: Tue, 01 Mar 2016 18:57:50 GMT\r\n\
Accept-Ranges: bytes\r\n\
Content-Length: %li\r\n\
Content-Type: text/html\r\n\r\n", st.st_size);
	write(connfd, buffer, strlen(buffer));
	sendfile(connfd, f, NULL, st.st_size);
	close(f);
}


void *pthread_main(void *socketfd)
{
	int serverfd = *((int *)socketfd);
	int connfd;
	while(1){
		connfd = acceptClient(serverfd);
		func(connfd);
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

	printf("\n%s\n%s\n%s\n%s\n",route, numCli, numPort, name);
	info = initserverSocket(LISTEN, atoi(numPort));
	if (!info)
		exit(-1);


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
