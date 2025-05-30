#include <reg52.h>
#include <math.h>

// 硬件定义
#define FOSC 11059200L
#define LED_PORT P2
sbit K1 = P1^4; // 按键K1连接到P1_4
sbit K2 = P1^5; // 按键K2连接到P1_5

// 全局变量
unsigned int timer_count = 0;
unsigned char duty_cycle[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char direction = 1;
const float gamma = 0.3; // 典型的伽马值 2.2
unsigned int flow_speed = 5; // 定义流动速度变量，值越大速度越慢 50

// 伽马校正函数
// 功能：对输入的亮度值进行伽马校正
// 参数：value - 输入的亮度值，范围0 - 255
// 返回值：校正后的亮度值，范围0 - 255
unsigned char gamma_correct(unsigned char value) {
    float corrected = 255.0 * pow((float)value / 255.0, gamma);
    return (unsigned char)corrected;
}

// 定时器0初始化（1ms中断）
// 功能：初始化定时器0为模式1，设置初值，使能中断并启动定时器
void Timer0_Init() {
    // 设置定时器0为模式1（16位定时器/计数器）
    TMOD |= 0x01;
    // 计算并设置定时器0高8位初值，用于1ms定时
    TH0 = (65536 - FOSC / 12 / 1000) >> 8;
    // 计算并设置定时器0低8位初值，用于1ms定时
    TL0 = (65536 - FOSC / 12 / 1000) & 0xFF;
    ET0 = 1; // 使能定时器0中断
    TR0 = 1; // 启动定时器0
    EA = 1;  // 使能总中断
}

// 定时器0中断服务函数
// 功能：处理定时器0的中断，实现LED亮度的动态调整和流动效果
void Timer0_ISR() interrupt 1 {
    unsigned char i; // 在函数开头声明i

    // 重新加载定时器0高8位初值
    TH0 = (65536 - FOSC / 12 / 1000) >> 8;
    // 重新加载定时器0低8位初值
    TL0 = (65536 - FOSC / 12 / 1000) & 0xFF;

    timer_count++;
    if (timer_count >= flow_speed) {
        timer_count = 0;
        for (i = 0; i < 8; i++) {
            if (duty_cycle[i] < 100) {
                duty_cycle[i]++;
            }
        }
        if (direction) {
            if (duty_cycle[7] >= 100) {
                direction = 0;
            }
        } else {
            if (duty_cycle[0] >= 100) {
                direction = 1;
            }
        }
    }

    for (i = 0; i < 8; i++) {
        unsigned char corrected_duty = gamma_correct(duty_cycle[i] * 2.55);
        if (timer_count <= corrected_duty) {
            LED_PORT |= (1 << i);
        } else {
            LED_PORT &= ~(1 << i);
        }
    }
}

// 主函数
// 功能：初始化定时器并进入无限循环，处理按键事件以调整占空比
void main() {
    unsigned char i; // 在函数开头声明i

    // 设置P0、P1、P3端口默认低电平
    P0 = 0x00;
  //  P1 = 0x00;
    P3 = 0x00;

    Timer0_Init();

    while (1) {
        if (K1 == 0) { // K1按键按下
            for (i = 0; i < 8; i++) {
                if (duty_cycle[i] < 90) {
                    duty_cycle[i] += 10;
                }
            }
            while (K1 == 0); // 等待按键释放
        }
        if (K2 == 0) { // K2按键按下
            for (i = 0; i < 8; i++) {
                if (duty_cycle[i] > 10) {
                    duty_cycle[i] -= 10;
                }
            }
            while (K2 == 0); // 等待按键释放
        }
    }
}