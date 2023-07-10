#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int sendRelayCommand(int sockfd, int relayNum, int state) {
    unsigned char request[] = {
        0xCC, 0xDD, 0xA1,  // 帧头
        0x01,              // 地址
        0x00, 0x00,        // 控制位
        0x00, 0x00,        // 使能位
        0x00, 0x00         // 校验（先填充���0）
    };

    if (relayNum < 1 || relayNum > 16) {
        printf("Invalid relay number\n");
        return -1;
    }

    // 计算控制位
    unsigned int controlBit = 1 << (relayNum - 1);

    request[4] = request[6] = controlBit >> 8;
    request[5] = request[7] = controlBit & 0xFF;
    if(!state)
    {
        request[4] = request[5] = 0x00;
    }
    // 计算校验
    request[8] = request[2] + request[3] + request[4] + request[5] + request[6] + request[7];
    request[9] = (request[8] +request[8] )& 0xFF;  // 取低8位

    ssize_t numBytesSent = send(sockfd, request, sizeof(request), 0);
    if (numBytesSent < 0) {
        perror("Send failed");
        return -1;
    }
    for(int i=0;i<10;i++)
    {
        printf("%02X",request[i]);
    }
    printf("\r\n");
    printf("Relay %d %s command sent successfully\n", relayNum, state ? "ON" : "OFF");

    return 0;
}

int main(int argc, char **argv) {
    // 参数检查
    if (argc != 3) {
        printf("Usage: %s num \"on\" or \"off\"\n", argv[0]);
        return -1;
    }

    // 初始化
    int sockfd;
    struct sockaddr_in serverAddr;

    // 创建TCP套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // 设置服务器地址和端口
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(50000);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.0.18");

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return -1;
    }

    // 获取继电器编号和状态
    int relayNum = atoi(argv[1]);
    int state = strcmp(argv[2], "on") == 0 ? 1 : 0;

    // 发送继电器开关指令
    sendRelayCommand(sockfd, relayNum, state);

    // 接收响应
    unsigned char recvBuffer[BUFFER_SIZE];
    ssize_t numBytesRecv = recv(sockfd, recvBuffer, BUFFER_SIZE, 0);
    if (numBytesRecv < 0) {
        perror("Receive failed");
        close(sockfd);
        return -1;
    }

    printf("Response received: ");
    for (int i = 0; i < numBytesRecv; i++) {
        printf("%02X ", recvBuffer[i]);
    }
    printf("\n");

    // 关闭套接字
    close(sockfd);

    return 0;
}