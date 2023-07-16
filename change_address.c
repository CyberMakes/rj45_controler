#include <modbus.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>


// 02 06 00 01 00 05 3A 18
int main(int argc, char *argv[])
{
    modbus_t *ctx;
    uint16_t tab_reg[25];
    int rc;
    int device_num = 2;
    char *device_value_name = NULL;

    // 创建Modbus RTU上下文
    ctx = modbus_new_rtu("/dev/ttyS9", 9600, 'N', 8, 1);
    if (ctx == NULL)
    {
        fprintf(stderr, "Failed to create the Modbus context\n");
        return -1;
    }

    // 设置命令码为6
    modbus_set_debug(ctx, TRUE);
    // 设置从机地址
    if (modbus_set_slave(ctx, device_num) == -1)
    {
        fprintf(stderr, "Failed to set Modbus slave address\n");
        modbus_free(ctx);
        return -1;
    }

    // 连接到从机
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Modbus connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    // 发送修改地址命令
    rc = modbus_write_register(ctx, 0x0001, 0x0006);
    if (rc == -1)
    {
        fprintf(stderr, "Modbus write failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

}