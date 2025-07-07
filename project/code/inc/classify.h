/*
 * @Author: wxq 13498874+wtbz0915@user.noreply.gitee.com
 * @Date: 2025-07-03 14:22:35
 * @LastEditors: wxq 13498874+wtbz0915@user.noreply.gitee.com
 * @LastEditTime: 2025-07-03 15:20:22
 * @FilePath: \project\code\inc\classify.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __CLASSIFY_H__
#define __CLASSIFY_H__

#include "zf_common_headfile.h"


#define PUSH_LEFT   0
#define PUSH_RIGHT  1
#define JIAN_GE  5


//三大类物品分类
typedef enum{
    Class_Utilities = 3,
    Class_Electronic_Peripherals = 2,
    Class_Num = 1,
}Class_Three_t;

//十五类物品分类
typedef enum{

    Class_Wrench        =   'e',    //扳手
    Class_Solder        =   'f',    //电烙铁
    Class_Drill         =   'g',    //手电钻
    Class_Tape          =   'h',    //卷尺
    Class_Screwdriver   =   'i',    //螺丝刀
    Class_Tongs         =   'j',    //钳子
    Class_Oscilloscope  =   'k',    //示波器
    Class_Multimeter    =   'l',    //万用表

    Class_Printer       =   'm',    //打印机
    Class_Keyboard      =   'n',    //键盘
    Class_Phone         =   'o',    //手机
    Class_Mouse         =   'p',    //鼠标
    Class_Headphone     =   'q',    //头戴式耳机
    Class_Monitor       =   'r',    //显示器
    Class_Sound         =   's',    //音响  

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
void result_diaplay(uint8_t Class_Number);
#endif 