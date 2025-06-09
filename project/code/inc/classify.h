#ifndef __CLASSIFY_H__
#define __CLASSIFY_H__

#include "zf_common_headfile.h"


#define PUSH_LEFT   0
#define PUSH_RIGHT  1
//��������Ʒ����
typedef enum{
    Class_Utilities = 3,
    Class_Electronic_Peripherals = 2,
    Class_Num = 1,
}Class_Three_t;

//ʮ������Ʒ����
typedef enum{

    Class_Wrench        =   'd',    //����
    Class_Solder        =   'e',    //������
    Class_Drill         =   'f',    //�ֵ���
    Class_Tape          =   'g',    //���
    Class_Screwdriver   =   'h',    //��˿��
    Class_Tongs         =   'i',    //ǯ��
    Class_Oscilloscope  =   'j',    //ʾ����
    Class_Multimeter    =   'k',    //���ñ�

    Class_Printer       =   'l',    //��ӡ��
    Class_Keyboard      =   'm',    //����
    Class_Phone         =   'n',    //�ֻ�
    Class_Mouse         =   'o',    //���
    Class_Headphone     =   'p',    //ͷ��ʽ����
    Class_Monitor       =   'q',    //��ʾ��
    Class_Sound         =   'r',    //����  

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

#endif 