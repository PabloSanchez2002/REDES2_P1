/**
 * @file wrapper.c
 * @author Álvaro Rodríguez, Pablo Sanchez
 * @brief 
 * @version 0.1
 * @date 2023-02-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "../inc/p1.h"
/**
 * @brief Creates a socket
 * 
 * @return socket file descriptor if everything went ok, BASIC_ERROR if not.
 */
int createSocket()
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd == -1) {
        return BASIC_ERROR;
	}
	else
		syslog(LOG_USER,"Creating socket:      \033[1;92m✓\033[0;39m\n");
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        return BASIC_ERROR;
    return socketfd;
}

/**
 * @brief Binds the socket to the localip and the port specified in PORT
 * 
 * @param socketfd file descriptor of the socket to bind
 * @return OK or ERROR
 */
int bindSocket(int socketfd, int port)
{
    struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if ((bind(socketfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        return BASIC_ERROR;
	}
	else
		syslog(LOG_USER,"Binding socket:       \033[1;92m✓\033[0;39m\n");
    return OK;
}
/**
 * @brief Makes the socket start listening, up to MAX_LISTENERS at a time
 * 
 * @param socketfd  file descriptor of the socket to listen
 * @return OK or ERROR
 */
int listenSocket(int socketfd)
{
	if ((listen(socketfd, MAX_LISTENERS)) != 0) {
        return BASIC_ERROR;
	}
	else
		syslog(LOG_USER,"Starting to listen:   \033[1;92m✓\033[0;39m\n");

    return OK;

}
/**
 * @brief Accepts a new client from the listener queue
 * 
 * @param socketfd  the file descriptor of the socket where we listening
 * @return the file descriptor of the client or ERROR
 */
int acceptClient(int socketfd)
{
    int len;
    int socketconn;
	struct sockaddr_in cli;
    len = sizeof(cli);

	syslog(LOG_USER,"Accepting client:     "); 
    fflush(stdout);
	socketconn = accept(socketfd, (SA*)&cli, &len);
	if (socketconn < 0) {
		syslog(LOG_USER,"\033[0;91m✕\033[0;39m\n");
        return BASIC_ERROR;
	}
	else
		syslog(LOG_USER,"\033[1;92m✓\033[0;39m\n");
    return socketconn;

}
/**
 * @brief makes a client connect a socket
 * 
 * @param socketfd  the socket where he will connect
 * @return OK or ERROR
 */
int connectSocket(int socketfd, int port)
{

	struct sockaddr_in servaddr;

	bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(port);

	// connect the client socket to server socket
	if (connect(socketfd, (SA*)&servaddr, sizeof(servaddr))!= 0) {
        return BASIC_ERROR;
	}
	else
		syslog(LOG_USER,"Connecting to socket: \033[1;92m✓\033[0;39m\n");
    return OK;
}
/**
 * @brief this function will call up to 4 functions depending on the flag, in order to open a socket, bind it, listen and accept clients.
 * Based on the option flag, the fucntion will create, create and bind, create bind and listen, create bind listen and accept or create and connect. 
 * Example:
 * server: serverSocket(ACCEPT)
 * client: serverSocket(CLIENT)
 * with only this 2 calls a connexion between a server and client is made.
 * 
 * If we just wanted to bind we would call:
 * serverSocket(BIND) --> this will create the socket and bind it to our ip.
 * 
 * @param flag The value of flag is specified in p1.h (CLIENT, CREATE, BIND, LISTEN, ACCEPT) -> (1,2,3,4,5)
 * @return an array with two values: if NULL -> there was an error allocating memory. Else, the errors are stored in ret[1], and they depend on the function that gave error
 * flags are specified in p1.h (CREATESOCKETERR, BINDSOCKETERR, LISTENSOCKETERR, ACCEPTSOCKETERR, CONNECTSOCKETERR) ->(-2, -3, -4, -5, -6)
 * If everything went okey it will store: ret[0] = our socketfd, ret[1] = client socket fd.
 * If flag is for example BIND -> Accept isnt called, then ret[1] will have a 0 value.
 */
int *serverSocket(int flag, int port)
{

    int *ret = (int *) malloc(sizeof(int)*3);
    if (!ret)
        return NULL;
    ret[0] = 0;
    ret[1] = 0;

	if((ret[0] = createSocket(AF_INET, SOCK_STREAM, 0)) == -1)//-1 error else fd
    {
        ret[1] = CREATESOCKETERR;
        return ret;
    }
    
    if (flag >= BIND && (bindSocket(ret[0], port) == -1))//0 on success -1 else
    {
        ret[1] = BINDSOCKETERR;
        return ret;
    }

    if (flag >= LISTEN && listenSocket(ret[0]) == -1)//0 on success -1 else
    {
        ret[1] = LISTENSOCKETERR;
        return ret;
    }
    if ( flag == ACCEPT && (ret[1] = acceptClient(ret[0])) == -1)//-1 error else fd
    {
        ret[1] = ACCEPTSOCKETERR;
        return ret;
    }
    if ( flag == CLIENT && (connectSocket(ret[0], port) == -1))
    {
        ret[1] = CONNECTSOCKETERR;
        return ret;
    }
	return ret;
}
/**
 * @brief  closes the socket file descriptor specified in info[0] and then frees info
 * 
 * @param info 
 * @return OK or ERROR 
 */
int freeSocket(int *info)
{
    if (!info)
    {
        syslog(LOG_USER,"\033[0;91mError: \033[0;39mNull pointer\n");
        return BASIC_ERROR;
    }
    close(info[0]);
    free(info);
    return OK;
}
/**
 * @brief This function will call serverSocket and handle the posible errors it can give before returning.
 * 
 * @param flag The value of flag is specified in p1.h (CLIENT, CREATE, BIND, LISTEN, ACCEPT) -> (1,2,3,4,5)
 * @return NULL if error or the file descriptors correctly opened
 */
int *initserverSocket(int flag, int port)
{
    int *ret;
    if (flag < 1 || flag > 5)
    {
        syslog(LOG_USER,"\033[0;91mError: \033[0;39m Wrong Flag\n");
        return NULL;
    }
    if(!(ret=serverSocket(flag, port)))
    {
        syslog(LOG_USER,"\033[0;91mError: \033[0;39m Couldn't allocate memory\n");
        return NULL;
    }
    switch (ret[1])
    {
    case CREATESOCKETERR:
        syslog(LOG_USER,"\033[0;91mError: \033[0;39mCouldn't create socket\n");
        free(ret);
        return NULL;
    case BINDSOCKETERR:
        syslog(LOG_USER,"\033[0;91mError: \033[0;39mCouldn't bind socket\n");
        close(ret[0]);
        free(ret);
        return NULL;
    case LISTENSOCKETERR:
        syslog(LOG_USER,"\033[0;91mError: \033[0;39mCouldn't start listening\n");
        close(ret[0]);
        free(ret);
        return NULL;
    case ACCEPTSOCKETERR:
        syslog(LOG_USER,"\033[0;91mError: \033[0;39mCouldn't accept client\n");
        close(ret[0]);
        free(ret);
        return NULL;
    case CONNECTSOCKETERR:
        syslog(LOG_USER,"\033[0;91mError: \033[0;39mCouldn't connect to socket\n");
        close(ret[0]);
        free(ret);
        return NULL;
    default:
        syslog(LOG_USER,"\033[1;92mEverything worked fine\033[0;39m\n");
        return ret;
    }
}