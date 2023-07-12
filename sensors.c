#include <stdio.h>
#include <modbus.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // 参数检查
    if (argc != 2)
    {
        printf("Usage: %s [device_num]\n", argv[0]);
        return -1;
    }

    modbus_t *ctx;
    uint16_t tab_reg[1];
    int rc;
    int device_num = 0;
    char *device_value_name = NULL;

    // device_num转为16进制
    device_num = atoi(argv[1]) & 0xff;
    ctx = modbus_new_rtu("/dev/ttyS9", 9600, 'N', 8, 1);
    if (ctx == NULL)
    {
        fprintf(stderr, "Failed to create the Modbus context\n");
        return -1;
    }

    if (modbus_set_slave(ctx, device_num) == -1)
    {
        fprintf(stderr, "Failed to set Modbus slave address\n");
        modbus_free(ctx);
        return -1;
    }

    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Modbus connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    // 根据设备类型，设置读取的寄存器起始地址
    int start_address = 0;
    if (device_num == 0x03)
    {
        start_address = 0x0002; // 光照传感器寄存器起始地址
        rc = modbus_read_registers(ctx, start_address, 2, tab_reg);
        if (rc == -1)
        {
            fprintf(stderr, "Modbus read failed: %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return -1;
        }
        goto print_value;
    }
    else if (device_num == 0x04)
    {
        start_address = 0x0000; // 雨雪传感器寄存器起始地址
    }

    rc = modbus_read_registers(ctx, start_address, 1, tab_reg);
    if (rc == -1)
    {
        fprintf(stderr, "Modbus read failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

print_value:
    switch (device_num)
    {
    case 0x01:
        device_value_name = "噪声值";
        printf("%s: %.1f dB\n", device_value_name, (float)tab_reg[0] / 10.0);
        break;
    case 0x02:
        device_value_name = "风速值";
        printf("%s: %.1f m/s\n", device_value_name, (float)tab_reg[0] / 10.0);
        break;
    case 0x03:
        device_value_name = "光照值";
        uint32_t light_value = tab_reg[0] << 16 | tab_reg[1];
        printf("%s: %d Lux\n", device_value_name, light_value);
        break;
    case 0x04:
        device_value_name = "雨雪值";
        if (tab_reg[0] == 0x00)
        {
            printf("%s: 正常\n", device_value_name);
        }
        else if (tab_reg[0] == 0x01)
        {
            printf("%s: 报警\n", device_value_name);
        }
        else
        {
            printf("%s: 未知状态\n", device_value_name);
        }
        break;
    default:
        device_value_name = "未知值";
        printf("%s: %d\n", device_value_name, tab_reg[0]);
        break;
    }

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}