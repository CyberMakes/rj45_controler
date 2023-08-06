#include <pthread.h>
#include "tcpServer.h"
#include "rs485Sensors.h"

int main()
{
    // 创建服务器线程
    pthread_t serverThread;
    if(pthread_create(&serverThread, NULL, server_thread, NULL) != 0)
    {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    // 创建传感器线程
    pthread_t sensorThread;
    if (pthread_create(&sensorThread, NULL, sensor_thread, (void *)client_sockets) != 0)
    {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
    pthread_join(sensorThread, NULL);
    pthread_join(serverThread, NULL);
    return 0;
}