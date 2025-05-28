#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "zf_common_headfile.h"

#define BUTTON1_PIN D17
#define BUTTON2_PIN D16

#define PUSH_DOWN_LEVEL 0

#define BUTTON_SHORTPRESS 0x01
#define BUTTON_LONGPRESS  0x02

typedef struct Button{
    gpio_pin_enum BUTTON_PIN;

    uint16_t Denoi_tick;        //消抖动计时
    uint16_t lock_flag;         //锁定标志
    uint8_t long_press_flag;   //长按标志

    uint8_t button_status;    //按键状态 同时需要短按 长按时使用
    
}button_t;



#endif