
#include "../inc/p1.h"
#include "../inc/main.h"
#include "../inc/picohttpparser.h"

// Function designed for chat between client and server.
void func(int connfd)
{
	char buf[4096];
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
	//printf("\n ###########Path is: x%.*sx", (int)path_len, path);
	char *addr, source[80] = "templates";

	if ((int)path_len > 1)
	{
		printf("Nos vamos mi rey\n");
		printf("\n\nsource:%s\n\n", source);
		printf("\n\nPath:%.*s\n\n", (int)path_len, path);
		addr = (char *)malloc((int)path_len);
		memcpy(addr, path, (int)path_len);
		strcat(source, addr);
		f = open(source, O_RDONLY);
	}
	else
	{
		printf("Nos quedamos mi rey\n");
		f = open("templates/index.html", O_RDONLY);
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

	func(connfd);
}


void *pthread_main(void *socketfd)
{
	int serverfd = *((int *)socketfd);
	int connfd;
	connfd = acceptClient(serverfd);
	func(connfd);
}

int main()
{

	int *info;
	info = initserverSocket(LISTEN);
	if (!info)
		exit(0);

	/*pthread_t *threads;
	threads = (pthread_t*) malloc(sizeof(pthread_t)*5);
	if ((mutex = sem_open("mutex", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED)
	{
		perror("sem_create");
		exit(EXIT_FAILURE);
	}

	for(int i = 0;i<5;i++)
	{
		pthread_create(&threads[i], NULL,pthread_main, (void *) &info[0]);
	}
	for(int i=0;i<5;i++)
	{
		pthread_join(threads[i], NULL);
	}
	sem_close(mutex);
	sem_unlink("mutex");
	free(threads);*/

	func(acceptClient(info[0]));
	freeSocket(info);
}
