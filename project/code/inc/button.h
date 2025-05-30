#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "zf_common_headfile.h"

#define BUTTON1_PIN D17
#define BUTTON2_PIN D16

#define PUSH_DOWN_LEVEL 0

#define BUTTON_SHORTPRESS 0x01
#define BUTTON_LONGPRESS  0x02

#define BUTTON_IDLE		0x00
#define BUTTON_TICK		0x01
#define BUTTON_FLIT		0x02
#define BUTTON_LONG		0x03

typedef struct Button{
    gpio_pin_enum BUTTON_PIN;

    uint16_t Denoi_tick;        //��������ʱ
    uint16_t lock_flag;         //������־

    uint8_t button_status;    //����״̬ ͬʱ��Ҫ�̰� ����ʱʹ��
    
}button_t;

uint8_t button_scan_pushdown(button_t* button, uint16_t tick);
uint8_t button_scan_longshort(button_t* button, uint16_t long_tick, uint16_t short_tick);

#endif