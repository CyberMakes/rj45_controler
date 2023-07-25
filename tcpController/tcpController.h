#ifndef __TCPCONTROLLER_H__
#define __TCPCONTROLLER_H__

enum tcpController {
    CONTROL = 0xA1, // 控制继电器
    READRELAY = 0xB0, //读取继电器状态
    PULSE_OUTPUT = 0x33, // 电动脉冲输出 0x33或0x35
    SEQUENCE_CONDUCT = 0x44, // 顺序导通 0x44
    SEQUENCE_BREAK = 0x55, // 顺序断开 0x55
    READSWITCH = 0xC0, // 读取开关量
};

void jsonParse(char *jsonStr);

#endif // !__TCPCONTROLLER_H__