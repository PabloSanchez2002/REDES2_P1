/**
 * @brief Implements the functionality associated with the client (not used).
 *
 * @file client.c
 * @author Alvaro Rodriguez & Pablo Sanchez
 * @version 1.0
 * @date 13/03/2023
 * @copyright GNU Public License
 */
#include "../inc/p1.h"

void func(int sockfd)
{
	char buff[MAX];
	int n;
	for (;;) {
		bzero(buff, sizeof(buff));
		printf("Enter the string : ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n')
			;
		write(sockfd, buff, sizeof(buff));
		bzero(buff, sizeof(buff));
		read(sockfd, buff, sizeof(buff));
		printf("From Server : %s", buff);
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
	}
}

int main()
{
	int *info;
	info = initserverSocket(CLIENT, 8080);
	if (!info)
		exit(0);
	func(info[0]);
	freeSocket(info);
}