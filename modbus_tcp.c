#include <stdio.h>
#include <modbus.h>
#include "errno.h"

int main(void) {
    modbus_t *ctx = NULL;
    uint8_t request[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x05, 0x00, 0x00, 0xFF, 0x00};
    uint8_t response[32] = {0};

    // 创建 Modbus TCP 上下文
    ctx = modbus_new_tcp("192.168.0.18", 50000);
    if (ctx == NULL) {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }

    // 连接到 Modbus TCP 服务器
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    // 设置从机地址
    modbus_set_slave(ctx, 1);

    // 发送 Modbus 命令，并接收响应
    int rc = modbus_send_raw_request(ctx, request, 12);
    if (rc == -1) {
        fprintf(stderr, "Send request failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    rc = modbus_receive_confirmation(ctx, (uint8_t *)response);
    if (rc == -1) {
        fprintf(stderr, "Receive response failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    // 打印响应数据
    printf("Response: %d\n", response[0]);

    // 关闭连接和释放资源
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
