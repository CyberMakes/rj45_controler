#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <modbus.h>
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
#include "tcpController.h"
#include "rs485Sensors.h"
#include <pthread.h>

#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024
#define CONTROIP1 "192.168.10.20"
#define CONTROIP2 "192.168.10.21"
#define CONTROPORT 50000

extern int tcpControllerSockList[MAX_CLIENTS];
extern char *tcpControllerIPList[MAX_CLIENTS];
extern char *sensor_data;
#endif // !__MAIN_H

