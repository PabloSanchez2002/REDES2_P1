
#include "../inc/p1.h"
#include "../inc/main.h"

// Function designed for chat between client and server.
void func(int connfd)
{
	char buff[MAX];
	int n;
	// infinite loop for chat
	for (;;) {
		bzero(buff, MAX);

		// read the message from client and copy it in buffer
		read(connfd, buff, sizeof(buff));
		// print buffer which contains the client contents
		printf("From client: %s\t To client : ", buff);
		bzero(buff, MAX);
		n = 0;
		// copy server message in the buffer
		while ((buff[n++] = getchar()) != '\n')
			;

		// and send that buffer to client
		write(connfd, buff, sizeof(buff));

		// if msg contains "Exit" then server exit and chat ended.
		if (strncmp("exit", buff, 4) == 0) {
			printf("Server Exit...\n");
			break;
		}
	}
}

void *pthread_main(void *socketfd)
{
	int serverfd = *((int *)socketfd);
	int connfd;
	sem_wait(mutex);
	connfd = acceptClient(serverfd);
	sem_post(mutex);
	func(connfd);

}

int main()
{
	
	int *info;
	info = initserverSocket(LISTEN);
	if (!info)
		exit(0);
	
	pthread_t *threads;
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
	free(threads);
	
	freeSocket(info);
}
