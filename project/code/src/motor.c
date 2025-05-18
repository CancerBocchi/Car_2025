#include "motor.h"

//  
//			 |
//	 1	 |   2
//----------------
//			 |
//	 3	 |   4

//������ٱ�Ϊ70/14���������ٶȺ������ٶȵı���Ϊ70:30



// ���PID���ƿ�
Pos_PID_t Motor_PID_A;
Pos_PID_t Motor_PID_B;
Pos_PID_t Motor_PID_C;


// ���Ŀ���ٶ�
float MA_target_speed;
float MB_target_speed;
float MC_target_speed;

float Motor_A_Act_Speed;
float Motor_B_Act_Speed;
float Motor_C_Act_Speed;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ������к���
//  @param      void
//  @return     void
//  @e.g.       ��Ƶ1k
//-------------------------------------------------------------------------------------------------------------------
void motor_run()
{
		gpio_toggle_level(B9);
		encoder_getvalue();
		Motor_Pwm_cb();
		gpio_toggle_level(B9);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ����߳����
//  @param      void
//  @return     void
//  @e.g.       ��������ʱ��ʹ��
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
//  @brief      ����̳߳�ʼ��
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
//  @brief      电机用pit中断 1khz
//  @param      void
//  @return     void
//  @e.g.       使用pit ch1
//-------------------------------------------------------------------------------------------------------------------
void motor_pit_init()
{
	
	pit_init(MOTOR_PIT_CHANNEL, 74999);

}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      �����ʼ��
//  @param      void
//  @return     void
//  @e.g.       
//-------------------------------------------------------------------------------------------------------------------
void Motor_init()
{

	rt_kprintf("Motor Init\n");

	encoder_init();

	//pwm��ʼ��
	// m1 D0Ϊ PH D1Ϊ EN    				
  	pwm_init(MOTORA_PWM, 20000, 0);
	gpio_init(MOTORA_DIR,GPO,0,GPO_PUSH_PULL);
	
	Pos_PID_Init(&Motor_PID_A,30,0.5,0);
	Motor_PID_A.Ref = 0;
	Motor_PID_A.Output_Max = 9500;
	Motor_PID_A.Output_Min = -9500;
	Motor_PID_A.Value_I_Max = 2000;
	MA_target_speed = 0;

	// m2 D3Ϊ EN D2Ϊ PH     				
  	pwm_init(MOTORB_PWM, 20000, 0);
	gpio_init(MOTORB_DIR,GPO,0,GPO_PUSH_PULL);
	
	Pos_PID_Init(&Motor_PID_B,-30,-0.5,0);
	Motor_PID_B.Ref = 0;
	Motor_PID_B.Output_Max = 9500;
	Motor_PID_B.Output_Min = -9500;
	Motor_PID_B.Value_I_Max = 2000;
	MB_target_speed = 0;

	// m3 D14Ϊ PH D15Ϊ EN
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
//  @brief      ����ջ�
//  @param      void
//  @return     void
//  @e.g.       Ƶ��1k����
//-------------------------------------------------------------------------------------------------------------------
void Motor_Pwm_cb()
{
	//����1000Ϊÿ�����е��ٶ�
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


