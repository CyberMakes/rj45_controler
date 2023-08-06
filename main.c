#include "main.h"

int tcpControllerSockList[MAX_CLIENTS];
char *tcpControllerIPList[MAX_CLIENTS] = {CONTROIP1, CONTROIP2};

// 获取传感器数据并发送的线程
void *sensor_reading_thread(void *arg)
{
    while (1)
    {
        // Read sensor data and send it to connected devices (excluding those in tcpControllerSockList)
        char *sensor_data = 0;
        sensor_data = (char *)malloc(4096);
        memset(sensor_data, 0, 4096);
        read_sensor(sensor_data);
        // printf("222sensor_data:%s\r\n", sensor_data);

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int *client_sockets = (int *)arg;
            printf("client_sockets:%d\r\n", client_sockets[i]);
            if (client_sockets[i] != 0)
            {
                send(client_sockets[i], sensor_data, strlen(sensor_data), 0);
            }
        }
        // Sleep for 5 seconds
        sleep(5);
        memset(sensor_data, 0, 4096);
    }
    if (sensor_data != NULL)
    {
        free(sensor_data);
    }
    return NULL;
}

// 建立TCP连接
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

static void tcpControllerSocketCreate(void)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (establishConnection(&tcpControllerSockList[i], tcpControllerIPList[i]) == 0)
        {
            printf("与%s连接成功\r\n", tcpControllerIPList[i]);
        }
    }
}

int main()
{
    int server_socket, client_sockets[MAX_CLIENTS], max_clients = MAX_CLIENTS;
    struct sockaddr_in server_addr, client_addr;
    fd_set readfds, allfds;
    int max_fd, activity, i, valread, new_socket, sd;
    char buffer[BUFFER_SIZE];
    socklen_t addrlen;

    // 与网口控制器连接
    tcpControllerSocketCreate();

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6666);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 5) == -1)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Initialize client_sockets array
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_sockets[i] = 0;
    }

    // Set the server socket in the allfds set
    FD_ZERO(&allfds);
    FD_SET(server_socket, &allfds);
    max_fd = server_socket;

    printf("Server listening on port 6666...\n");

    // 创建线程
    pthread_t sensor_thread;
    if (pthread_create(&sensor_thread, NULL, sensor_reading_thread, (void *)client_sockets) != 0)
    {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

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

    return 0;
}