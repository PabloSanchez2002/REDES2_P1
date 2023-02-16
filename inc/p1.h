/**
 * @file p1.h
 * @author Álvaro Rodríguez, Pablo Sanchez
 * @brief 
 * @version 0.1
 * @date 2023-02-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef P1_H_
#define P1_H_

#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>


#define MAX 8000
#define MAX_LISTENERS 5
#define PORT 8080
#define SA struct sockaddr

#define OK 0

#define BASIC_ERROR             -1
#define CREATESOCKETERR         -2
#define BINDSOCKETERR           -3
#define LISTENSOCKETERR         -4
#define ACCEPTSOCKETERR         -5
#define CONNECTSOCKETERR        -6

#define CLIENT 1
#define CREATE  2
#define BIND    3
#define LISTEN  4
#define ACCEPT  5

int *initserverSocket(int flag);
int freeSocket(int *info);
int createSocket();
int bindSocket(int socketfd);
int listenSocket(int socketfd);
int acceptClient(int socketfd);
int connectSocket(int socketfd);
#endif