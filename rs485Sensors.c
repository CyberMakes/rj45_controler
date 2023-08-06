#include <stdio.h>
#include <modbus.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "main.h"

#define DEVICE_NUM 9

typedef struct
{
    char *device_name;
    modbus_t *ctx;
    uint16_t start_address;
    uint16_t register_num;
} rs485_sensor;

rs485_sensor rs485_sensor_list[DEVICE_NUM] = {
    {"噪声传感器", 0, 0x0000, 1},     // 噪声传感器
    {"风速传感器", 0, 0x0000, 1},     // 风速传感器
    {"光照传感器", 0, 0x0002, 2},     // 光照传感器
    {"雨雪传感器", 0, 0x0000, 1},     // 雨雪传感器
    {"空气质量传感器", 0, 0x0000, 7}, // 空气质量传感器
    {"红外传感器", 0, 0x0006, 1},     // 红外传感器
    {"电表", 0, 0x0007, 1},           // 电表
    {"甲烷传感器", 0, 0x0008, 1},     // 甲烷传感器
    {"浸水传感器", 0, 0x0000, 1},     // 浸水传感器
};

modbus_t *connect_modbus(int device_num)
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
        // modbus_close(ctx);
        // modbus_free(ctx);
        // exit(1);
    }
    return 0;
}

void print_value(int device_num, uint16_t *tab_reg, char *sensor_data)
{
    char temp_str[1024]; // 临时缓冲区用于存储每次sprintf的结果
    char *device_value_name = rs485_sensor_list[device_num - 1].device_name;

    switch (device_num)
    {
    case 0x01:
    {

        sprintf(temp_str, "%s: %.1f dB\n", device_value_name, (float)tab_reg[0] / 10.0);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾
    }
    break;
    case 0x02:
    {
        device_value_name = "风速值";
        sprintf(temp_str, "%s: %.1f m/s\n", device_value_name, (float)tab_reg[0] / 10.0);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾
    }
    break;
    case 0x03:
    {
        device_value_name = "光照值";
        uint32_t light_value = tab_reg[0] << 16 | tab_reg[1];
        sprintf(temp_str, "%s: %d Lux\n", device_value_name, light_value);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾
    }
    break;
    case 0x04:
    {
        device_value_name = "雨雪值";
        if (tab_reg[0] == 0x00)
        {
            sprintf(temp_str, "%s: 正常\n", device_value_name);
            strcat(sensor_data, temp_str); // 追加到sensor_data末尾
        }
        else if (tab_reg[0] == 0x01)
        {
            sprintf(temp_str, "%s: 报警\n", device_value_name);
            strcat(sensor_data, temp_str); // 追加到sensor_data末尾
        }
        else
        {
            sprintf(temp_str, "%s: 未知状态\n", device_value_name);
            strcat(sensor_data, temp_str); // 追加到sensor_data末尾
        }
    }
    break;
    case 0x05:
    {
        device_value_name = "空气质量值";
        sprintf(temp_str, "%s CO2:%uppm, CH2O:%uug/m3, TVOC:%uug/m3, PM2.5:%uug/m3, PM10:%uug/m3, T:%.2f℃, H:%.2fRH\n",
                device_value_name,
                tab_reg[0],
                tab_reg[1],
                tab_reg[2],
                tab_reg[3],
                tab_reg[4],
                (float)tab_reg[5] / 100.0,
                (float)tab_reg[6] / 100.0);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾
    }
    break;
    case 0x06:
    {
        device_value_name = "红外感应值";
        if (tab_reg[0] == 0x00)
        {
            sprintf(temp_str, "%s: 无人\n", device_value_name);
            strcat(sensor_data, temp_str); // 追加到sensor_data末尾
        }
        else if (tab_reg[0] == 0x01)
        {
            sprintf(temp_str, "%s: 有人\n", device_value_name);
            strcat(sensor_data, temp_str); // 追加到sensor_data末尾
        }
        else
        {
            sprintf(temp_str, "%s: 未知状态\n", device_value_name);
            strcat(sensor_data, temp_str); // 追加到sensor_data末尾
        }
    }
    break;
    case 0x07:
    {
        int start_address = 0;

        start_address = 0x00; // 电压
        read_register(rs485_sensor_list[7].ctx, start_address, 1, tab_reg);
        sprintf(temp_str, "电压: %.1f V\n", (float)tab_reg[0] / 10.0);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾

        start_address = 0x03; // 电流
        read_register(rs485_sensor_list[7].ctx, start_address, 1, tab_reg);
        sprintf(temp_str, "电流: %.1f A\n", (float)tab_reg[0] / 10.0);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾

        start_address = 0x07; // 总有功功率
        read_register(rs485_sensor_list[7].ctx, start_address, 1, tab_reg);
        sprintf(temp_str, "总有功功率: %.1f W\n", (float)tab_reg[0] / 10.0);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾++++

        start_address = 0x0B; // 总无功功率
        read_register(rs485_sensor_list[7].ctx, start_address, 1, tab_reg);
        sprintf(temp_str, "总无功功率: %.1f W\n", (float)tab_reg[0] / 10.0);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾

        start_address = 0x0F; // 总视在功率
        read_register(rs485_sensor_list[7].ctx, start_address, 1, tab_reg);
        sprintf(temp_str, "总视在功率: %.1f W\n", (float)tab_reg[0] / 10.0);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾

        start_address = 0x13; // 总功率因数
        read_register(rs485_sensor_list[7].ctx, start_address, 1, tab_reg);
        sprintf(temp_str, "总功率因数: %.1f\n", (float)tab_reg[0] / 100.0);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾

        start_address = 0x1A; // 频率
        read_register(rs485_sensor_list[7].ctx, start_address, 1, tab_reg);
        sprintf(temp_str, "频率: %.1f Hz\n", (float)tab_reg[0] / 100.0);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾

        start_address = 0x1D; // 有功电能
        read_register(rs485_sensor_list[7].ctx, start_address, 1, tab_reg);
        sprintf(temp_str, "有功电能: %u.%u kWh\n", tab_reg[0], tab_reg[1]);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾

        start_address = 0x21; // 无功电能
        read_register(rs485_sensor_list[7].ctx, start_address, 1, tab_reg);
        sprintf(temp_str, "反向总有功电能: %u.%u kWh\n", tab_reg[0], tab_reg[1]);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾

        sleep(1);
    }
    break;
    case 0x08:
    {
        device_value_name = "甲烷值";
        sprintf(temp_str, "%s: %d LEL\n", device_value_name, tab_reg[0]);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾
    }
    break;
    case 0x09:
    {
        device_value_name = tab_reg[0] ? "浸水" : "正常";
        sprintf(temp_str, "%s\n", device_value_name);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾
    }
    break;
    default:
    {
        device_value_name = "未知值";
        sprintf(temp_str, "%s: %d\n", device_value_name, tab_reg[0]);
        strcat(sensor_data, temp_str); // 追加到sensor_data末尾
    }
    break;
    }
}

int read_sensor(char *sensor_data)
{
    uint16_t tab_reg[100] = {0};

    for (int i = 0; i < DEVICE_NUM; i++)
    {
        // 连接Modbus
        rs485_sensor_list[i].ctx = connect_modbus(i + 1);
        if (rs485_sensor_list[i].ctx != NULL)
        {
            printf("%s连接成功\n", rs485_sensor_list[i].device_name);
        }
        else
        {
            printf("%s连接失败\n", rs485_sensor_list[i].device_name);
        }
        // 读取寄存器
        read_register(rs485_sensor_list[i].ctx, rs485_sensor_list[i].start_address, rs485_sensor_list[i].register_num, tab_reg);

        print_value(i + 1, tab_reg, sensor_data);
        // 断开连接
        modbus_close(rs485_sensor_list[i].ctx);
        if (rs485_sensor_list[i].ctx != NULL)
        {
            modbus_free(rs485_sensor_list[i].ctx);
        }
    }
    // printf("111sensor_data: %s\n", sensor_data);

    return 0;
}

// QList(
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "噪声传感器"))("value", QVariant(QString, "42.5dB")))),
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "风速值"))("value", QVariant(QString, "0.0m/s")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "光照值"))("value", QVariant(QString, "28Lux")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "雨雪值"))("value", QVariant(QString, "正常")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "红外感应值"))("value", QVariant(QString, "无人")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "电压"))("value", QVariant(QString, "211.5V")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "电流"))("value", QVariant(QString, "4.5A")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "总有功功率"))("value", QVariant(QString, "0.0W")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "总无功功率"))("value", QVariant(QString, "24.7W")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "总视在功率"))("value", QVariant(QString, "13.2W")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "总功率因数"))("value", QVariant(QString, "10.0")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "频率"))("value", QVariant(QString, "50.0Hz")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "有功电能"))("value", QVariant(QString, "0.4kWh")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "反向总有功电能"))("value", QVariant(QString, "0.4kWh")))), 
//     QVariant(QVariantMap, QMap(("name", QVariant(QString, "甲烷值"))("value", QVariant(QString, "0LEL"))))
//     )