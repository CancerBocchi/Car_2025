/*
 * @Author: wxq 13498874+wtbz0915@user.noreply.gitee.com
 * @Date: 2025-07-03 14:22:35
 * @LastEditors: wxq 13498874+wtbz0915@user.noreply.gitee.com
 * @LastEditTime: 2025-07-03 15:20:22
 * @FilePath: \project\code\inc\classify.h
 * @Description: ����Ĭ������,������`customMade`, ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __CLASSIFY_H__
#define __CLASSIFY_H__

#include "zf_common_headfile.h"


#define PUSH_LEFT   0
#define PUSH_RIGHT  1
#define JIAN_GE  5


//��������Ʒ����
typedef enum{
    Class_Utilities = 3,
    Class_Electronic_Peripherals = 2,
    Class_Num = 1,
}Class_Three_t;

//ʮ������Ʒ����
typedef enum{

    Class_Wrench        =   'e',    //����
    Class_Solder        =   'f',    //������
    Class_Drill         =   'g',    //�ֵ���
    Class_Tape          =   'h',    //���
    Class_Screwdriver   =   'i',    //��˿��
    Class_Tongs         =   'j',    //ǯ��
    Class_Oscilloscope  =   'k',    //ʾ����
    Class_Multimeter    =   'l',    //���ñ�

    Class_Printer       =   'm',    //��ӡ��
    Class_Keyboard      =   'n',    //����
    Class_Phone         =   'o',    //�ֻ�
    Class_Mouse         =   'p',    //���
    Class_Headphone     =   'q',    //ͷ��ʽ����
    Class_Monitor       =   'r',    //��ʾ��
    Class_Sound         =   's',    //����  

}Class_Fifteen_t;

//�洢��Ʒ��Ϣ�Ľṹ��
typedef struct{

    uint8_t         num;            //���
    Class_Three_t   Class_Huge;     //��Ʒ�����
    uint8_t         Class_Small;    //��ƷС����
    uint8_t         Class_Flag;     //�Ƿ��Ѿ�����

}Class_Info_t;

void Class_Init();
void Class_Debug();
uint8_t Class_Add(uint8_t Class);
void result_diaplay(uint8_t Class_Number);
#endif 