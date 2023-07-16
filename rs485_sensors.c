#include <stdio.h>
#include <modbus.h>
#include <errno.h>
#include <stdlib.h>

int device_num;
char *device_value_name;

modbus_t *connect_modbus(int device_num)
{
    int rc;
    modbus_t *ctx;

    // 创建Modbus RTU上下文
    ctx = modbus_new_rtu("/dev/ttyS9", 9600, 'N', 8, 1);
    if (ctx == NULL)
    {
        fprintf(stderr, "Failed to create the Modbus context\n");
        exit(1);// 退出程序
    }

    // 设置从机地址
    if (modbus_set_slave(ctx, device_num) == -1)
    {
        fprintf(stderr, "Failed to set Modbus slave address\n");
        modbus_free(ctx);
        exit(1);// 退出程序
    }

    // 连接到从机
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Modbus connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        exit(1);// 退出程序
    }
    return ctx;
}

int read_register(modbus_t *ctx, int start_address, int register_num, uint16_t *tab_reg)
{
    int rc = modbus_read_registers(ctx, start_address, register_num, tab_reg);
    if (rc != register_num)
    {
        fprintf(stderr, "Failed to read: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        modbus_free(ctx);
        exit(1);
    }
    return 0;
}

void print_value(int device_num, uint16_t *tab_reg)
{
    switch (device_num)
    {
    case 0x01:
    {
        device_value_name = "噪声值";
        printf("%s: %.1f dB\n", device_value_name, (float)tab_reg[0] / 10.0);
    }
    break;
    case 0x02:
    {
        device_value_name = "风速值";
        printf("%s: %.1f m/s\n", device_value_name, (float)tab_reg[0] / 10.0);
    }
    break;
    case 0x03:
    {
        device_value_name = "光照值";
        uint32_t light_value = tab_reg[0] << 16 | tab_reg[1];
        printf("%s: %d Lux\n", device_value_name, light_value);
    }
    break;
    case 0x04:
    {
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
    }
    break;
    case 0x05:
    {
        device_value_name = "空气质量值";
        printf("%s CO2:%uppm, CH2O:%uug/m3, TVOC:%uug/m3, PM2.5:%uug/m3, PM10:%uug/m3, T:%.2f℃, H:%.2fRH\n",
               device_value_name,
               tab_reg[0],
               tab_reg[1],
               tab_reg[2],
               tab_reg[3],
               tab_reg[4],
               (float)tab_reg[5] / 100.0,
               (float)tab_reg[6] / 100.0);
    }
    break;
    case 0x06:
    {
        device_value_name = "红外感应值";
        if (tab_reg[0] == 0x00)
        {
            printf("%s: 无人\n", device_value_name);
        }
        else if (tab_reg[0] == 0x01)
        {
            printf("%s: 有人\n", device_value_name);
        }
        else
        {
            printf("%s: 未知状态\n", device_value_name);
        }
    }
    break;
    default:
        device_value_name = "未知值";
        printf("%s: %d\n", device_value_name, tab_reg[0]);
        break;
    }
}

int main(int argc, char *argv[])
{
    // 参数检查
    if (argc != 2)
    {
        printf("Usage: %s [device_num]\n", argv[0]);
        return -1;
    }
    uint16_t tab_reg[100];
    device_num = atoi(argv[1]);
    if (device_num < 1 || device_num > 7)
    {
        printf("device_num must be 1~7\n");
        return -1;
    }

    // 连接Modbus
    modbus_t *ctx = connect_modbus(device_num);

    // 根据设备类型，设置读取的寄存器起始地址
    int start_address = 0;
    int register_num = 0;
    switch (device_num)
    {
    case 0x03:
        start_address = 0x0002; // 温湿度传感器寄存器起始地址
        register_num = 2;
        break;
    case 0x05:
        register_num = 7;
    case 0x06:
        start_address = 0x0006; // 红外感应传感器寄存器起始地址
        break;
    case 0x07:
    {
        for (int i = 1; i < 10; i++)
        {
            switch (i)
            {
            case 1:
                start_address = 0x00; // 电压
                read_register(ctx, start_address, 1, tab_reg);
                printf("电压: %.1f V\n", (float)tab_reg[0] / 10.0);
                break;
            case 2:
                start_address = 0x03; // 电流
                read_register(ctx, start_address, 1, tab_reg);
                printf("电流: %.1f A\n", (float)tab_reg[0] / 10.0);
                break;
            case 3:
                start_address = 0x07; // 总有功功率
                read_register(ctx, start_address, 1, tab_reg);
                printf("总有功功率: %.1f W\n", (float)tab_reg[0] / 10.0);
                break;
            case 4:
                start_address = 0x0B; // 总无功功率
                read_register(ctx, start_address, 1, tab_reg);
                printf("总无功功率: %.1f W\n", (float)tab_reg[0] / 10.0);
                break;
            case 5:
                start_address = 0x0F; // 总视在功率
                read_register(ctx, start_address, 1, tab_reg);
                printf("总视在功率: %.1f W\n", (float)tab_reg[0] / 10.0);
                break;
            case 6:
                start_address = 0x13; // 总功率因数
                read_register(ctx, start_address, 1, tab_reg);
                printf("总功率因数: %.1f\n", (float)tab_reg[0] / 100.0);
                break;
            case 7:
                start_address = 0x1A; // 频率
                read_register(ctx, start_address, 1, tab_reg);
                printf("频率: %.1f Hz\n", (float)tab_reg[0] / 10.0);
                break;
            case 8:
                start_address = 0x1D; // 有功电能
                read_register(ctx, start_address, 2, tab_reg);
                printf("有功电能: %u.%u kWh\n", tab_reg[0], tab_reg[1]);
                break;
            case 9:
                start_address = 0x21; // 无功电能
                read_register(ctx, start_address, 2, tab_reg);
                printf("反向总有功电能: %u.%u kWh\n", tab_reg[0], tab_reg[1]);
                break;
            default:
                break;
            }
        }
        goto end;
    }
    break;
    default:
        start_address = 0x0000; // 噪声传感器寄存器起始地址
        register_num = 1;
        break;
    }

    // 读取寄存器
    read_register(ctx, start_address, register_num, tab_reg);

    print_value(device_num, tab_reg);
end:
    // 断开连接
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}