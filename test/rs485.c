#include <stdio.h>
#include <modbus.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define RS485_DEVICE_ADDRESS 0x50
#define REGISTER 0x0033
modbus_t *connect_modbus(uint8_t device_num)
{
    modbus_t *ctx;
    // 创建Modbus RTU上下文
    ctx = modbus_new_rtu("/dev/ttyS9", 9600, 'N', 8, 1);
    if (ctx == NULL)
    {
        fprintf(stderr, "Failed to create the Modbus context\n");
        // exit(1); // 退出程序
    }
    // modbus_set_debug(ctx, TRUE);

    // 设置从机地址
    if (modbus_set_slave(ctx, device_num) == -1)
    {
        fprintf(stderr, "Failed to set Modbus slave address\n");
        // modbus_free(ctx);
        // exit(1); // 退出程序
    }

    // 连接到从机
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Modbus connection failed: %s\n", modbus_strerror(errno));
        // modbus_free(ctx);
        // exit(1); // 退出程序
    }
    return ctx;
}

int read_register(modbus_t *ctx, int start_address, int register_num, uint16_t *tab_reg)
{
    int rc = modbus_read_registers(ctx, start_address, register_num, tab_reg);
    if (rc != register_num)
    {
        fprintf(stderr, "Failed to read: %s\n", modbus_strerror(errno));
        return -1;
    }
    return 0;
}

int write_register(modbus_t *ctx, int start_address, int register_num, uint16_t *tab_reg)
{
    int rc = modbus_write_registers(ctx, start_address, register_num, tab_reg);
    if (rc != register_num)
    {
        fprintf(stderr, "Failed to write: %s\n", modbus_strerror(errno));
        return -1;
    }
    return 0;
}
int main()
{

    uint16_t tab_reg[100] = {0};

    // 连接Modbus
    modbus_t *ctx = connect_modbus(RS485_DEVICE_ADDRESS);
    if (ctx != NULL)
    {
        // printf("连接成功\n");
    }
    else
    {
        printf("连接失败\n");
    }
    // 写入寄存器
    // tab_reg[0] = 0x01;
    // if (write_register(ctx, REGISTER, 1, tab_reg) != 0)
    // {
    //     printf("写入失败\n");
    // }
    // else
    // {
    //     printf("写入成功\n");
    // }

    // 读取寄存器
    if (read_register(ctx, REGISTER, 1, tab_reg) != 0)
    {
        printf("读取失败\n");
    }
    else
    {
        printf("读取成功\n");
        // 打印寄存器值
        printf("tab_reg[0] = %02x\n", tab_reg[0]);
    }

    // 断开连接
    modbus_close(ctx);
    if (ctx != NULL)
    {
        modbus_free(ctx);
    }

    return 0;
}
