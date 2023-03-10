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

#include <arpa/inet.h> 
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> 
#include <sys/socket.h>
#include <unistd.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <syslog.h>
#include <time.h>

#define MAX 8000
#define MAX_STRING 1024
#define MAX_CHAR 20

#define BIG_STRING 8192
#define MEDIUM_STRING 1024
#define SMALL_STRING 128
#define MAX_LISTENERS 5
#define PORT 8080
#define SA struct sockaddr

#define SMALL_ARR 32
#define MEDIUM_ARR 128
#define BIG_ARR 512

#define PY "python3"
#define PHP "php"

#define PY_EXTENSION ".py"
#define PHP_EXTENSION ".php"

#define PHP_NUM 0
#define PY_NUM 1

#define READ 0
#define WRITE 1


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

#define GET "GET"
#define POST "POST"
#define OPTIONS "OPTIONS"

#define PIPE_TAM 2


typedef enum s_status
{
    OK = 1,
    ERROR = 0
} t_status;

typedef enum s_bool
{
    TRUE = 1,
    FALSE = 0
} t_bool;


int *initserverSocket(int flag, int port);
int freeSocket(int *info);
int createSocket();
int bindSocket(int socketfd, int port);
int listenSocket(int socketfd);
int acceptClient(int socketfd);
int connectSocket(int socketfd, int port);
#endif