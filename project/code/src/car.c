#include "car.h"


//	����  �ұ�Ϊ��x ���Ϊ��x
//	  	|||||||||	
//		|		|	|
//		|  che	|	|
//		|		| -------->x
//		|charger|	|
//		|		|	|
//		|||||||||	+y
//

mecanum_Speed Car_Speed;

Pos_PID_t Car_Yaw_Controller;

uint8_t Car_BootSwitch = 1;//�����ܿ���

//
//�����ٶȿ���Ȩ��
Speed_Con_Right Car_Speed_ConRight;

float linearSpeed = 1000;
float angularSpeed = 1000;
float maxLinearSpeed = 1000;
float maxAngularSpeed = 200;
float minLinearSpeed = 100;
float minAngularSpeed = 300;

/**
 * @brief ���ֽ���
 * 
 * @param xSpeed x�����ٶ�
 * @param ySpeed y�����ٶ�
 * @param aSpeed z�����ٶ�
 */
void mecanumRun(float xSpeed, float ySpeed, float aSpeed)
{
    float speedA = - xSpeed*SIN30 + ySpeed/COS30 + aSpeed;
    float speedB = - xSpeed*SIN30 - ySpeed/COS30 + aSpeed;
    float speedC = xSpeed + aSpeed;

    float max = speedA;
    if (max < speedB)   max = speedB;
    if (max < speedC)   max = speedC;
		
    if (max > maxLinearSpeed){
        speedA = speedA / max * maxLinearSpeed;
        speedB = speedB / max * maxLinearSpeed;
        speedC = speedC / max * maxLinearSpeed;
    }
		
	Motor_Set_Speed(1,speedA);
	Motor_Set_Speed(2,-speedB);
	Motor_Set_Speed(3,speedC);
		
}

/**
 * @brief �����ٶȸı�ӿ�
 * 
 * @param xSpeed x�����ٶ�
 * @param ySpeed y�����ٶ�
 * @param aSpeed ���ٶ�
 * 			
 */
void Car_Change_Speed(float xSpeed, float ySpeed, float aSpeed)
{
	if(Car_BootSwitch){
		Car_Speed.Vx = xSpeed;
		Car_Speed.Vy = ySpeed;

		if(Car_Speed_ConRight != Con_By_AngleLoop)
			Car_Speed.Omega = aSpeed;
	}
	else
		return;

}

void Car_Change_Yaw(float Yaw_Ref)
{	
	Car_Yaw_Controller.Ref = Yaw_Ref;
}

void car_motion_run()
{
	//gpio_toggle_level(B9);

	// Att_GetYaw();
	
	//�Ƕȱջ� 
	if(Car_Speed_ConRight == Con_By_AngleLoop)
		Car_Speed.Omega = Pos_PID_Controller(&Car_Yaw_Controller,Att.yaw);

	//麦轮解算
	mecanumRun(Car_Speed.Vx,Car_Speed.Vy,Car_Speed.Omega);
	//驱动电机
	motor_run();

	//gpio_toggle_level(B9);
}

/**
 * @brief ���������߳�
 * 
 */
void car_motion_entry()
{
	while(1)
	{
		gpio_toggle_level(C3);
		Att_GetYaw();
		if(Car_BootSwitch)
			car_motion_run();
		gpio_toggle_level(C3);
		rt_thread_delay(1);
	}
}

/**
 *	@brief ����ת�亯��
 *			
 *	@param angle ת��Ƕ� ��ֵ��ת ������ת
 */
void Car_Rotate(float angle)
{
	Car_Speed_ConRight = Con_By_AngleLoop;
	
	Car_Change_Yaw(Att.yaw + angle);
}

/**
 * @brief �������к���
 * 
 * @param dx x��ȷ���ľ���	��λcm
 * @param dy y��ȷ���ľ���  ��λcm
 * @param dt �ﵽ�����ʱ�� ��λs
 */
void Car_DistanceMotion(float dx,float dy,float dt){
	
	float Vx = Car_DisConvert_x(dx)/dt;
	float Vy = Car_DsiConvert_y(dy)/dt;
	Vx = Vx>Car_Max_Speed? Car_Max_Speed/5:Vx;
	Vy = Vy>Car_Max_Speed? Car_Max_Speed/5:Vy;

	Car_Change_Speed(Vx,Vy,Car_Speed.Omega);
	//��ʱ
	rt_thread_delay((int)(dt*1000));
	//�ָ��ٶ�
	Car_Change_Speed(0,0,Car_Speed.Omega);
	rt_thread_delay(5);
}
	
/**
 * @brief �������Ƴ�ʼ��
 * 
 */
rt_thread_t car_motion_thread;

void car_motion_Init()
{
	rt_kprintf("Car Hardware init\n");
	//�����ʼ��
	Motor_init();

	//��̬�����ʼ��                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                Att_Algo_Init();
	Att_Algo_Init();
	
	//�������ƿ��ʼ��
	Car_Speed_ConRight = Con_By_TraceLine;
	Pos_PID_Init(&Car_Yaw_Controller,5.0,0,0);
	Car_Yaw_Controller.Ref = 0;
	Car_Yaw_Controller.Output_Max = 300;
	Car_Yaw_Controller.Output_Min = -300;
	Car_Yaw_Controller.Value_I_Max = 1000;


	//�̳߳�ʼ��
#if MOTOR_LOOP_METHOD == MOTOR_USE_CAR_LOOP
	#if CAR_LOOP_METHOD == CAR_USE_THREAD
	
		car_motion_thread = rt_thread_create("car_motion_thread",car_motion_entry,NULL,4096*2,1,1000);
	
		if(car_motion_thread != NULL)
		{
				rt_kprintf("Car Init Successfully!\n");
				rt_thread_startup(car_motion_thread);
		}
		else
			rt_kprintf("car_motion_thread created failed!\n");

	#elif CAR_LOOP_METHOD == CAR_USE_PIT

		pit_init(CAR_PIT_CHANNEL,74999);

	#endif

#endif

	Car_Speed.Omega = 0.0f;
	Car_Speed.Vx = 0.0f;
	Car_Speed.Vy = 0.0f;
		
	Car_BootSwitch = 0;
	Car_Stop();
	//Car_Speed_ConRight = Con_By_AngleLoop;
}

/**
 * @brief ����ֹͣ�������Ƚ����ٶ����㣬����
*/
void Car_Stop()
{
	Car_Switch(0);
	pwm_set_duty(MOTORA_PWM,0);
	pwm_set_duty(MOTORB_PWM,0);
	pwm_set_duty(MOTORC_PWM,0);
}

/**
 * @brief ������������
*/
void Car_Start()
{
	Car_Switch(1);
}

/**
 * @brief �����ٶȿ���
*/

