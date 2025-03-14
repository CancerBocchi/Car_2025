#ifndef _MOTOR_H
#define _MOTOR_H

#include "encoder.h"
#include "zf_common_headfile.h"
#include "PID.h"

// 锟斤拷锟轿伙拷锟斤拷锟酵硷拷锟绞?1?7
//			 |
//	 1	 |   2
//----------------
//			 |
//	 3	 |   4


// 鐢垫満妗嗘灦
//
//     [MOTOR PID]
//     [Encoder][PWM]
//


/*
* 电机闭环控制选项 常规的闭环使用车辆环路就行
*	前两者用于调试使用
*
*/
#define MOTOR_THREAD 			0	//基于单独的线程
#define MOTOR_PIT    			1	//基于定时器中断
#define MOTOR_USE_CAR_LOOP	 	2	//基于车辆环路


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


//电机运行函数
void motor_run();
//电机初始化
void Motor_init();
//对外接口
void Motor_Set_Speed(uint8_t Motor_CH,float target_speed);


//电机内部函数
void motor_pit_init();
void Motor_Pwm_cb();

void MotorA_Set_Pwm(int pwm);
void MotorB_Set_Pwm(int pwm);
void MotorC_Set_Pwm(int pwm);

#endif