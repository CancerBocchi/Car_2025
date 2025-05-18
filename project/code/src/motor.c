#include "motor.h"

//  
//			 |
//	 1	 |   2
//----------------
//			 |
//	 3	 |   4

//电机减速比为70/14，编码器速度和轮子速度的比例为70:30



// 电机PID控制块
Pos_PID_t Motor_PID_A;
Pos_PID_t Motor_PID_B;
Pos_PID_t Motor_PID_C;


// 电机目标速度
float MA_target_speed;
float MB_target_speed;
float MC_target_speed;

float Motor_A_Act_Speed;
float Motor_B_Act_Speed;
float Motor_C_Act_Speed;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机运行函数
//  @param      void
//  @return     void
//  @e.g.       定频1k
//-------------------------------------------------------------------------------------------------------------------
void motor_run()
{
		gpio_toggle_level(B9);
		encoder_getvalue();
		Motor_Pwm_cb();
		gpio_toggle_level(B9);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机线程入口
//  @param      void
//  @return     void
//  @e.g.       仅供调试时候使用
//-------------------------------------------------------------------------------------------------------------------
void motor_entry()
{
    while(1)
    {
        motor_run();
		//gpio_toggle_level(B9);
		rt_thread_delay(1);
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机线程初始化
//  @param      void
//  @return     void
//  @e.g.       
//-------------------------------------------------------------------------------------------------------------------
rt_thread_t motor_thread;
void motor_thread_init()
{
	motor_thread = rt_thread_create("motor",motor_entry,NULL,4096,1,1000);
    
    if(motor_thread!=NULL)
    {
		//rt_kprintf("motor_thread created successful!");
    	rt_thread_startup(motor_thread);
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      鐢垫満鐢╬it涓柇 1khz
//  @param      void
//  @return     void
//  @e.g.       浣跨敤pit ch1
//-------------------------------------------------------------------------------------------------------------------
void motor_pit_init()
{
	
	pit_init(MOTOR_PIT_CHANNEL, 74999);

}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机初始化
//  @param      void
//  @return     void
//  @e.g.       
//-------------------------------------------------------------------------------------------------------------------
void Motor_init()
{

	rt_kprintf("Motor Init\n");

	encoder_init();

	//pwm初始化
	// m1 D0为 PH D1为 EN    				
  	pwm_init(MOTORA_PWM, 20000, 0);
	gpio_init(MOTORA_DIR,GPO,0,GPO_PUSH_PULL);
	
	Pos_PID_Init(&Motor_PID_A,30,0.5,0);
	Motor_PID_A.Ref = 0;
	Motor_PID_A.Output_Max = 9500;
	Motor_PID_A.Output_Min = -9500;
	Motor_PID_A.Value_I_Max = 2000;
	MA_target_speed = 0;

	// m2 D3为 EN D2为 PH     				
  	pwm_init(MOTORB_PWM, 20000, 0);
	gpio_init(MOTORB_DIR,GPO,0,GPO_PUSH_PULL);
	
	Pos_PID_Init(&Motor_PID_B,-30,-0.5,0);
	Motor_PID_B.Ref = 0;
	Motor_PID_B.Output_Max = 9500;
	Motor_PID_B.Output_Min = -9500;
	Motor_PID_B.Value_I_Max = 2000;
	MB_target_speed = 0;

	// m3 D14为 PH D15为 EN
	pwm_init(MOTORC_PWM, 20000, 0);      				
	gpio_init(MOTORC_DIR,GPO,0,GPO_PUSH_PULL);

	Pos_PID_Init(&Motor_PID_C,30,0.5,0);
	Motor_PID_C.Ref = 0;
	Motor_PID_C.Output_Max = 9500;
	Motor_PID_C.Output_Min = -9500;
	Motor_PID_C.Value_I_Max = 2000;
	MC_target_speed = 0;
	
	pwm_set_duty(MOTORA_PWM,0);
	pwm_set_duty(MOTORB_PWM,0);
	pwm_set_duty(MOTORC_PWM,0);

#if MOTOR_LOOP_METHOD == MOTOR_THREAD

	motor_thread_init();
#elif MOTOR_LOOP_METHOD == MOTOR_PIT

	motor_pit_init();
#endif 

}


//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机闭环
//  @param      void
//  @return     void
//  @e.g.       频率1k运行
//-------------------------------------------------------------------------------------------------------------------
void Motor_Pwm_cb()
{
	//乘以1000为每秒运行的速度
	Motor_A_Act_Speed = RC_encoder1 * 1000 / 1024 * 3.14  * 6.1;
	Motor_B_Act_Speed = RC_encoder2 * 1000 / 1024 * 3.14  * 6.1;
	Motor_C_Act_Speed = RC_encoder3 * 1000 / 1024 * 3.14  * 6.1;

	MotorA_Set_Pwm( (int)Pos_PID_Controller(&Motor_PID_A,Motor_A_Act_Speed) );
	MotorB_Set_Pwm( (int)Pos_PID_Controller(&Motor_PID_B,Motor_B_Act_Speed) );
	MotorC_Set_Pwm( (int)Pos_PID_Controller(&Motor_PID_C,Motor_C_Act_Speed) );
	
}


float Slew_Func(float *slewVal, float refVal, float slewRate)
{
  static float diff = 0;
  diff = refVal - *slewVal;
  if (diff >= slewRate)
  {
    *slewVal += slewRate;
    return (1);
  } else if (-diff >= slewRate)
  {
    *slewVal -= slewRate;
    return (-1);
  } else
  {
    *slewVal = refVal;
    return (0);
  }
}

void MotorA_Set_Pwm(int pwm)
{
	Slew_Func(&Motor_PID_A.Ref,MA_target_speed,MOTOR_SOFTBOOT_RATE);
	MOTORA_CHECK_DIR(pwm);
	pwm_set_duty(MOTORA_PWM,abs(pwm));
	
}

void MotorB_Set_Pwm(int pwm)
{
	Slew_Func(&Motor_PID_B.Ref,MB_target_speed,MOTOR_SOFTBOOT_RATE);
	MOTORB_CHECK_DIR(pwm);
	pwm_set_duty(MOTORB_PWM,abs(pwm));

}

void MotorC_Set_Pwm(int pwm)
{
	Slew_Func(&Motor_PID_C.Ref,MC_target_speed,MOTOR_SOFTBOOT_RATE);
	MOTORC_CHECK_DIR(pwm);
	pwm_set_duty(MOTORC_PWM,abs(pwm));
	
}


void Motor_Set_Speed(uint8_t Motor_CH,float target_speed)
{
	if(Motor_CH == 1) 
	{
		MA_target_speed = target_speed;
	}
	else if(Motor_CH == 2)
	{
		MB_target_speed = target_speed;
	}
	else if(Motor_CH == 3)
	{
		MC_target_speed = target_speed;
	}
}


