#include <reg52.h>
#include <math.h>

// Ӳ������
#define FOSC 11059200L
#define LED_PORT P2
sbit K1 = P1^4; // ����K1���ӵ�P1_4
sbit K2 = P1^5; // ����K2���ӵ�P1_5

// ȫ�ֱ���
unsigned int timer_count = 0;
unsigned char duty_cycle[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char direction = 1;
const float gamma = 0.3; // ���͵�٤��ֵ 2.2
unsigned int flow_speed = 5; // ���������ٶȱ�����ֵԽ���ٶ�Խ�� 50

// ٤��У������
// ���ܣ������������ֵ����٤��У��
// ������value - ���������ֵ����Χ0 - 255
// ����ֵ��У���������ֵ����Χ0 - 255
unsigned char gamma_correct(unsigned char value) {
    float corrected = 255.0 * pow((float)value / 255.0, gamma);
    return (unsigned char)corrected;
}

// ��ʱ��0��ʼ����1ms�жϣ�
// ���ܣ���ʼ����ʱ��0Ϊģʽ1�����ó�ֵ��ʹ���жϲ�������ʱ��
void Timer0_Init() {
    // ���ö�ʱ��0Ϊģʽ1��16λ��ʱ��/��������
    TMOD |= 0x01;
    // ���㲢���ö�ʱ��0��8λ��ֵ������1ms��ʱ
    TH0 = (65536 - FOSC / 12 / 1000) >> 8;
    // ���㲢���ö�ʱ��0��8λ��ֵ������1ms��ʱ
    TL0 = (65536 - FOSC / 12 / 1000) & 0xFF;
    ET0 = 1; // ʹ�ܶ�ʱ��0�ж�
    TR0 = 1; // ������ʱ��0
    EA = 1;  // ʹ�����ж�
}

// ��ʱ��0�жϷ�����
// ���ܣ�����ʱ��0���жϣ�ʵ��LED���ȵĶ�̬����������Ч��
void Timer0_ISR() interrupt 1 {
    unsigned char i; // �ں�����ͷ����i

    // ���¼��ض�ʱ��0��8λ��ֵ
    TH0 = (65536 - FOSC / 12 / 1000) >> 8;
    // ���¼��ض�ʱ��0��8λ��ֵ
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

// ������
// ���ܣ���ʼ����ʱ������������ѭ�����������¼��Ե���ռ�ձ�
void main() {
    unsigned char i; // �ں�����ͷ����i

    // ����P0��P1��P3�˿�Ĭ�ϵ͵�ƽ
    P0 = 0x00;
  //  P1 = 0x00;
    P3 = 0x00;

    Timer0_Init();

    while (1) {
        if (K1 == 0) { // K1��������
            for (i = 0; i < 8; i++) {
                if (duty_cycle[i] < 90) {
                    duty_cycle[i] += 10;
                }
            }
            while (K1 == 0); // �ȴ������ͷ�
        }
        if (K2 == 0) { // K2��������
            for (i = 0; i < 8; i++) {
                if (duty_cycle[i] > 10) {
                    duty_cycle[i] -= 10;
                }
            }
            while (K2 == 0); // �ȴ������ͷ�
        }
    }
}