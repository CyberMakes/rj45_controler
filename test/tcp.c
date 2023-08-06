#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include "stdio.h"
#include "pthread.h"

#define CLIENTIP "192.168.10.5"
#define SERVERIP "192.168.10.6"
#define CLIENTPORT 9999
#define SERVERPORT 6666

void *client_thread()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Socket failed");
        return NULL;
    }

    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENTPORT);
    client_addr.sin_addr.s_addr = inet_addr(CLIENTIP);

    if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("Connection failed");
        close(sockfd);
        return NULL;
    }

    uint8_t data[100] = {0};
    printf("Please input data:\r\n");
    while (scanf("%s", data) != EOF)
    {
        size_t len = strlen(data);
        if (len % 2 != 0)
        {
            printf("Input data length must be even.\r\n");
            continue;
        }

        uint8_t hex_data[len / 2];
        for (size_t i = 0; i < len; i += 2)
        {
            sscanf(data + i, "%2hhx", &hex_data[i / 2]);
        }

        printf("Send: ");
        for (size_t i = 0; i < len / 2; i++)
        {
            printf("%02x ", hex_data[i]);
        }
        printf("\r\n");

        send(sockfd, hex_data, len / 2, 0);
    }
}

void *server_thread()
{
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Socket failed");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVERIP);

    if (bind(server_socket, (struct sockaddr *)&server_addr, addr_len) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        return NULL;
    }

    if (listen(server_socket, 5) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        return NULL;
    }

    while (1)
    {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        // printf("Accept: %d\r\n", client_socket);
        if (client_socket < 0)
        {
            perror("Accept failed");
            close(server_socket);
            return NULL;
        }

        uint8_t buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        ssize_t recv_len = recv(client_socket, buffer, sizeof(buffer), 0);
        if (recv_len > 0)
        {
            printf("Received: ");
            for (ssize_t i = 0; i < recv_len; i++)
            {
                printf("%02x ", buffer[i]);
                // printf("\r\n");
                // printf("%c ", buffer[i]);
            }
            printf("\r\n");
        }
        // close(client_socket);
    }
}

int main()
{
    pthread_t server, client;
    pthread_create(&server, NULL, server_thread, NULL);
    pthread_create(&client, NULL, client_thread, NULL);
    pthread_join(server, NULL);
    pthread_join(client, NULL);
    return 0;
}