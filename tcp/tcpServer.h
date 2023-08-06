#ifndef __TCPSERVER_H
#define __TCPSERVER_H
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>

#define MAX_CLIENTS 2

extern int client_sockets[MAX_CLIENTS]; // TCP网口控制器客户端套接字列表

void *server_thread(void *arg);
#endif // !__MAIN_H

