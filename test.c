#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define IP_ADDR "192.168.0.18"
#define PORT 50000

int main()
{
    // 创建TCP套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    // 设置服务器地址和端口
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDR);
    serverAddr.sin_port = htons(PORT);
    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Connection failed");
        close(sockfd);
        return -1;
    }
    while (1)
    {
        unsigned char recvBuffer[BUFFER_SIZE];
        ssize_t numBytesRecv = recv(sockfd, recvBuffer, BUFFER_SIZE, 0);
        if (numBytesRecv < 0)
        {
            perror("Read states failed");
            close(sockfd);
            return -1;
        }
        // 打印接收到的数据
        printf("Response received: ");
        for (int i = 0; i < numBytesRecv; i++)
        {
            printf("%02X ", recvBuffer[i]);
        }
        printf("\n");
    }

    // 关闭服务器套接字
    close(sockfd);

    return 0;
}
