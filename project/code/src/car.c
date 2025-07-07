#include "car.h"


//	车辆  右边为正x 左边为负x
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

uint8_t Car_BootSwitch = 1;//车辆总开关

//
//车辆速度控制权限
Speed_Con_Right Car_Speed_ConRight;

float linearSpeed = 1000;
float angularSpeed = 1000;
float maxLinearSpeed = 1000;
float maxAngularSpeed = 200;
float minLinearSpeed = 100;
float minAngularSpeed = 300;

/**
 * @brief 麦轮结算
 * 
 * @param xSpeed x方向速度
 * @param ySpeed y方向速度
 * @param aSpeed z方向速度
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
 * @brief 车辆速度改变接口
 * 
 * @param xSpeed x方向速度
 * @param ySpeed y方向速度
 * @param aSpeed 角速度
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
	
	//角度闭环 
	if(Car_Speed_ConRight == Con_By_AngleLoop)
		Car_Speed.Omega = Pos_PID_Controller(&Car_Yaw_Controller,Att.yaw);

	//楹﹁疆瑙ｇ畻
	mecanumRun(Car_Speed.Vx,Car_Speed.Vy,Car_Speed.Omega);
	//椹卞姩鐢垫満
	motor_run();

	//gpio_toggle_level(B9);
}

/**
 * @brief 车辆控制线程
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
 *	@brief 车辆转弯函数
 *			
 *	@param angle 转向角度 负值右转 正数左转
 */
void Car_Rotate(float angle)
{
	Car_Speed_ConRight = Con_By_AngleLoop;
	
	Car_Change_Yaw(Att.yaw + angle);
}

/**
 * @brief 车辆运行函数
 * 
 * @param dx x轴确定的距离	单位cm
 * @param dy y轴确定的距离  单位cm
 * @param dt 达到距离的时间 单位s
 */
void Car_DistanceMotion(float dx,float dy,float dt){
	
	float Vx = Car_DisConvert_x(dx)/dt;
	float Vy = Car_DsiConvert_y(dy)/dt;
	Vx = Vx>Car_Max_Speed? Car_Max_Speed/5:Vx;
	Vy = Vy>Car_Max_Speed? Car_Max_Speed/5:Vy;

	Car_Change_Speed(Vx,Vy,Car_Speed.Omega);
	//延时
	rt_thread_delay((int)(dt*1000));
	//恢复速度
	Car_Change_Speed(0,0,Car_Speed.Omega);
	rt_thread_delay(5);
}
	
/**
 * @brief 车辆控制初始化
 * 
 */
rt_thread_t car_motion_thread;

void car_motion_Init()
{
	rt_kprintf("Car Hardware init\n");
	//电机初始化
	Motor_init();

	//姿态结算初始化                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                Att_Algo_Init();
	Att_Algo_Init();
	
	//车辆控制块初始化
	Car_Speed_ConRight = Con_By_TraceLine;
	Pos_PID_Init(&Car_Yaw_Controller,5.0,0,0);
	Car_Yaw_Controller.Ref = 0;
	Car_Yaw_Controller.Output_Max = 300;
	Car_Yaw_Controller.Output_Min = -300;
	Car_Yaw_Controller.Value_I_Max = 1000;


	//线程初始化
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
 * @brief 车辆停止函数，先将车速度置零，上锁
*/
void Car_Stop()
{
	Car_Switch(0);
	pwm_set_duty(MOTORA_PWM,0);
	pwm_set_duty(MOTORB_PWM,0);
	pwm_set_duty(MOTORC_PWM,0);
}

/**
 * @brief 车辆启动函数
*/
void Car_Start()
{
	Car_Switch(1);
}

/**
 * @brief 车辆速度控制
*/

