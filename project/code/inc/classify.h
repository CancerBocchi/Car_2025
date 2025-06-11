#ifndef __CLASSIFY_H__
#define __CLASSIFY_H__

#include "zf_common_headfile.h"


#define PUSH_LEFT   0
#define PUSH_RIGHT  1
//三大类物品分类
typedef enum{
    Class_Utilities = 3,
    Class_Electronic_Peripherals = 2,
    Class_Num = 1,
}Class_Three_t;

//十五类物品分类
typedef enum{

    Class_Wrench        =   'd',    //扳手
    Class_Solder        =   'e',    //电烙铁
    Class_Drill         =   'f',    //手电钻
    Class_Tape          =   'g',    //卷尺
    Class_Screwdriver   =   'h',    //螺丝刀
    Class_Tongs         =   'i',    //钳子
    Class_Oscilloscope  =   'j',    //示波器
    Class_Multimeter    =   'k',    //万用表

    Class_Printer       =   'l',    //打印机
    Class_Keyboard      =   'm',    //键盘
    Class_Phone         =   'n',    //手机
    Class_Mouse         =   'o',    //鼠标
    Class_Headphone     =   'p',    //头戴式耳机
    Class_Monitor       =   'q',    //显示器
    Class_Sound         =   'r',    //音响  

}Class_Fifteen_t;

//存储物品信息的结构体
typedef struct{

    uint8_t         num;            //序号
    Class_Three_t   Class_Huge;     //物品大分类
    uint8_t         Class_Small;    //物品小分类
    uint8_t         Class_Flag;     //是否已经分类

}Class_Info_t;

void Class_Init();
void Class_Debug();
uint8_t Class_Add(uint8_t Class);

#endif 