#include <modbus.h>
#include "tcpController.h"
#include "rs485Sensors.h"
#include "main.h"
#include "tcpServer.h"

#define BUFFER_SIZE 1024
#define CONTRO_IP_1 "192.168.10.20"
#define CONTRO_IP_2 "192.168.10.21"
#define CONTROPORT 50000
#define SERVER_PORT 6666

int tcpControllerSockList[MAX_CLIENTS];                              // Tcp控制器套接字列表
char *tcpControllerIPList[MAX_CLIENTS] = {CONTRO_IP_1, CONTRO_IP_2}; // Tcp控制器IP列表
int client_sockets[MAX_CLIENTS];// TCP网口控制器客户端套接字列表

/// @brief: 连接到网口控制器
/// @param: sockfd: 套接字
/// @param: ip: IP地址
/// @attention: PORT已经通过CONTROLPORT宏定义为50000
/// @return: 成功返回0，失败返回-1
static int establishConnection(int *sockfd, const char *ip)
{
    struct sockaddr_in serverAddr;

    // 创建TCP套接字
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    // 设置服务器地址和端口
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(CONTROPORT);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    // 连接到服务器
    if (connect(*sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Connection failed");
        close(*sockfd);
        return -1;
    }

    return 0;
}

/// @brief: 创建TCP网口控制器套接字
static void tcpControllerSocketCreate(void)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (establishConnection(&tcpControllerSockList[i], tcpControllerIPList[i]) == 0)
        {
            // printf("与%s连接成功\r\n", tcpControllerIPList[i]);
        }
    }
}

/// @brief tcpserver线程函数
/// @return 
void *server_thread(void *arg)
{
    UNUSED(arg);
    int server_socket, max_clients = MAX_CLIENTS;
    struct sockaddr_in server_addr, client_addr;
    fd_set readfds, allfds;
    int max_fd, activity, i, valread, new_socket, sd;
    char buffer[BUFFER_SIZE];
    socklen_t addrlen;

    // 与网口控制器连接
    tcpControllerSocketCreate();

    // 创建服务器socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    // 设置服务器IP地址和端口
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // 绑定服务器socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听服务器socket
    if (listen(server_socket, 5) == -1)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // 初始化client_sockets数组
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_sockets[i] = 0;
    }

    // 初始化allfds
    FD_ZERO(&allfds);
    FD_SET(server_socket, &allfds);
    max_fd = server_socket;

    DEBUG_PRINT("Server listening on port 6666...\n");

    while (1)
    {
        // Copy allfds to readfds
        readfds = allfds;

        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL); // 第三个NULL表示阻塞

        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select error");
        }

        // Check for incoming connection on the server socket
        if (FD_ISSET(server_socket, &readfds))
        {
            addrlen = sizeof(client_addr);
            if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0)
            {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            // Add new_socket to the client_sockets array
            for (i = 0; i < max_clients; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    break;
                }
            }

            // Add new_socket to the allfds set
            FD_SET(new_socket, &allfds);

            // Update max_fd if necessary
            if (new_socket > max_fd)
            {
                max_fd = new_socket;
            }

            printf("New connection, socket fd is %d, IP is : %s, Port is : %d\n",
                   new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        // Check for data from clients
        for (i = 0; i < max_clients; i++)
        {
            sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds))
            {
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0)
                {
                    // Client disconnected
                    getpeername(sd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
                    printf("Host disconnected, IP is : %s, Port is : %d\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    // Close the socket and remove from allfds set
                    close(sd);
                    FD_CLR(sd, &allfds);
                    client_sockets[i] = 0;
                }
                else
                {
                    // Echo back the message to the client
                    buffer[valread] = '\0';
                    // send(sd, buffer, strlen(buffer), 0);
                    // printf("Received: %s\n", buffer);
                    jsonParse(buffer);
                }
            }
        }
    }

    // Close server socket
    close(server_socket);

    pthread_exit(NULL);
}