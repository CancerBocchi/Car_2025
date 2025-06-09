#include "classify.h"


static Class_Info_t Class_Info[100];
//已经分类了几个物品
uint8_t Class_Number = 0;

/**
 * @brief 结构体初始化
 * 
 */
void Class_Init(){
    for(int i = 0; i < 100; i++){
        Class_Info[i].Class_Flag = 0;
        Class_Info[i].num = 0;
    }
    Class_Number = 0;
}

/**
 * @brief 添加分类
 * 
 * @param Class 分类输入
 */
uint8_t Class_Add(uint8_t Class){
    //数字分类
    if(Class < Class_Wrench){
        Class_Info[Class_Number].Class_Flag     = 1;
        Class_Info[Class_Number].num            = Class_Number+1;
        Class_Info[Class_Number].Class_Huge     = Class_Num;
        Class_Info[Class_Number].Class_Small    = Class;
        Class_Number++;
        return (Class%2)?0:1;
    }
    //工具分类
    else{
        if(Class <= Class_Multimeter){
            Class_Info[Class_Number].Class_Flag     = 1;
            Class_Info[Class_Number].num            = Class_Number+1;
            Class_Info[Class_Number].Class_Huge     = Class_Utilities;
            Class_Info[Class_Number].Class_Small    = Class;
            Class_Number++;
            return 1;//右
        }
        else if(Class <= Class_Sound){
            Class_Info[Class_Number].Class_Flag     = 1;
            Class_Info[Class_Number].num            = Class_Number+1;
            Class_Info[Class_Number].Class_Huge     = Class_Electronic_Peripherals;
            Class_Info[Class_Number].Class_Small    = Class;
            Class_Number++;
            return 0;//左
        }
    }

}

/**
 * @brief 分类调试
 * 
 */
void Class_Debug(){
    for(int i = 0; i < Class_Number; i++){
        if(Class_Info[i].Class_Huge == Class_Num){
            rt_kprintf("Class:NO.%d, %d\n", Class_Info[i].num, Class_Info[i].Class_Small);
        }
        else{
            switch(Class_Info[i].Class_Small){
                case Class_Wrench:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Wrench");
                break;

                case Class_Multimeter:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Multimeter");
                break;

                case Class_Sound:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Sound");
                break;

                case Class_Headphone:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Headphone");
                break;

                case Class_Keyboard:
                    printf("Class:NO.%d, %s\n", Class_Info[i].num, "Keyboard");
                break;

                case Class_Mouse:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Mouse");
                break;

                case Class_Monitor:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Monitor");
                break;

                case Class_Oscilloscope:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Oscilloscope");
                break;

                case Class_Printer:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Printer");
                break;

                case Class_Screwdriver:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Screwdriver");
                break;

                case Class_Tape:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Tape");
                break;

                case Class_Tongs:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Tongs");
                break;

                case Class_Solder:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Solder");
                break;

                case Class_Drill:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Drill");
                break;

                case Class_Phone:
                    rt_kprintf("Class:NO.%d, %s\n", Class_Info[i].num, "Phone");
                break;

            }
        }
    }

}


