#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "zf_common_headfile.h"

#define BUTTON1_PIN D17
#define BUTTON2_PIN D16

typedef struct Button{
    gpio_pin_enum BUTTON_PIN;
    
}button;



#endif