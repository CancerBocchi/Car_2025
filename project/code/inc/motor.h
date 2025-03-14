#ifndef _MOTOR_H
#define _MOTOR_H

#include "encoder.h"
#include "zf_common_headfile.h"
#include "PID.h"

// ���λ����ͼ���?1?7
//			 |
//	 1	 |   2
//----------------
//			 |
//	 3	 |   4


// 电机框架
//
//     [MOTOR PID]
//     [Encoder][PWM]
//


/*
* ����ջ�����ѡ�� ����ıջ�ʹ�ó�����·����
*	ǰ�������ڵ���ʹ��
*
*/
#define MOTOR_THREAD 			0	//���ڵ������߳�
#define MOTOR_PIT    			1	//���ڶ�ʱ���ж�
#define MOTOR_USE_CAR_LOOP	 	2	//���ڳ�����·


extern Pos_PID_t Motor_PID_A;
extern Pos_PID_t Motor_PID_B;
extern Pos_PID_t Motor_PID_C;

extern float Motor_A_Act_Speed;
extern float Motor_B_Act_Speed;
extern float Motor_C_Act_Speed;

extern float MA_target_speed;
extern float MB_target_speed;
extern float MC_target_speed;


#define MOTORA_PWM PWM1_MODULE3_CHB_D1
#define MOTORA_DIR D0

#define MOTORB_PWM PWM2_MODULE3_CHB_D3
#define MOTORB_DIR D2

#define MOTORC_PWM PWM1_MODULE1_CHB_D15
#define MOTORC_DIR D14

#define MOTORA_CHECK_DIR(pwm) (gpio_set_level(MOTORA_DIR,(pwm>0)?1:0))
#define MOTORB_CHECK_DIR(pwm) (gpio_set_level(MOTORB_DIR,(pwm>0)?1:0))
#define MOTORC_CHECK_DIR(pwm) (gpio_set_level(MOTORC_DIR,(pwm>0)?1:0))


//������к���
void motor_run();
//�����ʼ��
void Motor_init();
//����ӿ�
void Motor_Set_Speed(uint8_t Motor_CH,float target_speed);


//����ڲ�����
void motor_pit_init();
void Motor_Pwm_cb();

void MotorA_Set_Pwm(int pwm);
void MotorB_Set_Pwm(int pwm);
void MotorC_Set_Pwm(int pwm);

#endif