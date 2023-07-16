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

// 发送继电器控制命令
int sendRelayCommand(int sockfd, int relayNum, int state)
{
    unsigned char request[] = {
        0xCC, 0xDD, 0xA1, // 帧头
        0x01,             // 地址
        0x00, 0x00,       // 控制位
        0x00, 0x00,       // 使能位
        0x00, 0x00        // 校验（先填充0）
    };

    if (relayNum < 1 || relayNum > 16)
    {
        if (relayNum == 0) // 0表示全部继电器
        {
            if (state)// 全部打开
            {
                request[4] = 0xFF;
                request[5] = 0xFF;
                request[6] = 0xFF;
                request[7] = 0xFF;
            }
            else// 全部关闭
            {
                request[4] = 0x00;
                request[5] = 0x00;
                request[6] = 0xFF;
                request[7] = 0xFF;
            }
            // 计算校验
            request[8] = request[2] + request[3] + request[4] + request[5] + request[6] + request[7];
            request[9] = (request[8] + request[8]) & 0xFF; // 取低8位

            ssize_t numBytesSent = send(sockfd, request, sizeof(request), 0);
            if (numBytesSent < 0)
            {
                perror("Send failed");
                return -1;
            }
            return 0;
        }
        else
        {
            printf("Invalid relay number\n");
            return -1;
        }
    }

    // 计算控制位
    unsigned int controlBit = 1 << (relayNum - 1);

    request[4] = request[6] = controlBit >> 8;
    request[5] = request[7] = controlBit & 0xFF;
    if (!state)//如果是关闭指令
    {
        request[4] = request[5] = 0x00;
    }
    // 计算校验
    request[8] = request[2] + request[3] + request[4] + request[5] + request[6] + request[7];
    request[9] = (request[8] + request[8]) & 0xFF; // 取低8位

    ssize_t numBytesSent = send(sockfd, request, sizeof(request), 0);
    if (numBytesSent < 0)
    {
        perror("Send failed");
        return -1;
    }

    // 接收响应
    unsigned char recvBuffer[BUFFER_SIZE];
    ssize_t numBytesRecv = recv(sockfd, recvBuffer, BUFFER_SIZE, 0);
    if (numBytesRecv < 0)
    {
        perror("Read response failed");
        close(sockfd);
        return -1;
    }

    // 转换为字符串
    char response[numBytesRecv * 2 + 1];
    for (int i = 0; i < numBytesRecv; i++)
    {
        sprintf(&response[i * 2], "%02X", recvBuffer[i]);
    }
    response[numBytesRecv * 2] = '\0';

    // 判断是否等于 "v1.0"  76 31 2E 30
    if (strcmp(response, "76312E30") == 0)
    {
        printf("Relay %d %s command sent successfully\n", relayNum, state ? "ON" : "OFF");
    }

    return 0;
}

// 读取继电器状态
int readRelayState(int sockfd)
{
    const unsigned char request[] = {0xCC, 0xDD, 0xB0, 0x01, 0x00, 0x00, 0x0D, 0xBE, 0x7C};// 读取继电器状态命令
    ssize_t numBytesSent = send(sockfd, request, sizeof(request), 0);// 发送命令
    if (numBytesSent < 0)
    {
        perror("Send failed");
        return -1;
    }

    // 接收 "v1.0" 响应 控制器接收到命令后会返回 "v1.0"  76 31 2E 30
    unsigned char versionResponse[4];
    ssize_t numVersionBytesRecv = recv(sockfd, versionResponse, 4, 0);// 接收 "v1.0" 响应
    if (numVersionBytesRecv < 0)
    {
        perror("Read version response failed");
        close(sockfd);
        return -1;
    }
    // 判断是否等于 "v1.0"  76 31 2E 30
    if (strcmp(versionResponse, "76312E30") == 0)
    {
        printf("read command sent successfully\n");
    }
    // 接收状态值 接收完 "v1.0" 响应后，接收继电器状态值
    unsigned char recvBuffer[BUFFER_SIZE];
    ssize_t numBytesRecv = recv(sockfd, recvBuffer, BUFFER_SIZE, 0);// 接收状态值
    if (numBytesRecv < 0)
    {
        perror("Read states failed");
        close(sockfd);
        return -1;
    }

    // printf("Response received: ");
    // for (int i = 0; i < numBytesRecv; i++)
    // {
    //     printf("%02X ", recvBuffer[i]);
    // }
    // printf("\n");

    // 解析响应数据
    uint16_t relayStates = (recvBuffer[4] << 8) | recvBuffer[5];
    printf("Relay states: ");
    for (int i = 0; i < 16; i++)
    {
        uint16_t state = (relayStates >> i) & 0x01;
        printf("%d is %s \r\n", i + 1, state ? "ON" : "OFF");
    }
    printf("\n");

    return 0;
}

// 建立ModbusTCP连接
int establishConnection(int *sockfd)
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
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDR);

    // 连接到服务器
    if (connect(*sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Connection failed");
        close(*sockfd);
        return -1;
    }

    return 0;
}

// 关闭连接
void closeConnection(int sockfd)
{
    // 关闭套接字
    close(sockfd);
}

int main(int argc, char **argv)
{
    // 参数检查
    if (argc == 2 && strcmp(argv[1], "read") == 0)
    {
        // 读取继电器状态
        int sockfd;
        if (establishConnection(&sockfd) == 0)
        {
            readRelayState(sockfd);
            closeConnection(sockfd);
        }
    }
    else if (argc == 3)
    {
        // 发送继电器控制命令
        int sockfd;
        if (establishConnection(&sockfd) == 0)
        {
            // 获取继电器编号和状态
            int relayNum = atoi(argv[1]);
            if (strcmp(argv[1], "all") == 0)
            {
                relayNum = 0;
            }
            int state = strcmp(argv[2], "on") == 0 ? 1 : 0;

            // 发送继电器开关指令
            sendRelayCommand(sockfd, relayNum, state);

            closeConnection(sockfd);
        }
    }
    else
    {
        printf("Usage: %s [1-16/all] on/off\n", argv[0]);
        return -1;
    }

    return 0;
}