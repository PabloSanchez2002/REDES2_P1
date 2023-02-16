/*
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include "../inc/p1.h"
#define MAX 10
#define PORT 8080
#define MAX_PTHREADS 5
#define SA struct sockaddr

// Function designed for chat between client and server.

void func(int connfd)
{
	char buff[MAX];
	int n;
	// infinite loop for chat
	for (;;)
	{
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
		if (strncmp("exit", buff, 4) == 0)
		{
			printf("Server Exit...\n");
			break;
		}
	}
}

// Driver function
int main()
{
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	sockfd = openSocketServer(PORT);
		
	
	connfd = listenServer(sockfd);

	// Function for chatting between client and server
	func(connfd);

	// After chatting close the socket
	close(sockfd);
}
*/
// Server side program that sends
// a 'hi client' message
// to every client concurrently

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>created..
#include <sys/types.h>
#include <unistd.h>

// PORT number
#define PORT 8080
#define MAX 10

void func(int connfd)
{
	char buff[MAX];
	int n;
	// infinite loop for chat
	for (;;)
	{
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
		if (strncmp("exit", buff, 4) == 0)
		{
			printf("Server Exit...\n");
			break;
		}
	}
}

int main()
{
	// Server socket id
	int sockfd, ret;

	// Server socket address structures
	struct sockaddr_in serverAddr;

	// Client socket id
	int clientSocket;

	// Client socket address structures
	struct sockaddr_in cliAddr;

	// Stores byte size of server socket address
	socklen_t addr_size;

	// Child process id
	pid_t childpid;

	// Creates a TCP socket id from IPV4 family
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// Error handling if socket id is not valid
	if (sockfd < 0)
	{
		printf("Error in connection.\n");
		exit(1);
	}

	printf("Server Socket is created.\n");

	// Initializing address structure with NULL
	memset(&serverAddr, '\0',
		   sizeof(serverAddr));

	// Assign port number created..and IP address
	// to the socket created
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);

	// 127.0.0.1 is a loopback address
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// Binding the socket id with
	// the socket structure
	ret = bind(sockfd,
			   (struct sockaddr *)&serverAddr,
			   sizeof(serverAddr));

	// Error handling
	if (ret < 0)
	{
		printf("Error in binding.\n");
		exit(1);
	}

	// Listening for connections (upto 10)
	if (listen(sockfd, 10) == 0)
	{
		printf("Listening...\n\n");
	}

	int cnt = 0;
	while (1)
	{

		// Accept clients and
		// store their information in cliAddr
		clientSocket = accept(
			sockfd, (struct sockaddr *)&cliAddr,
			&addr_size);

		// Error handling
		if (clientSocket < 0)
		{
			exit(1);
		}

		// Displaying information of
		// connected client
		printf("Connection accepted from %s:%d\n",
			   inet_ntoa(cliAddr.sin_addr),
			   ntohs(cliAddr.sin_port));

		// Print number of clients
		// connected till now
		printf("Clients connected: %d\n\n",
			   ++cnt);

		// Creates a child process
		if ((childpid = fork()) == 0)
		{

			// Closing the server socket id
			close(sockfd);

			// Send a confirmation message
			// to the client
			send(clientSocket, "hi client",
				 strlen("hi client"), 0);
		}
	}

	// Close the client socket id
	close(clientSocket);
	return 0;
}
