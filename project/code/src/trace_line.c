/***********************************************************************
* 一个环路控制平移速度，控制量为所有中点与图像中线的偏差量的平均值
* 另一个环路控制航向角，控制量为第一个块(LAST_OFFSET_POINT_NUM)与中点距离的平均值
* 参考链接：https://www.bilibili.com/video/BV1ZT4y1D716/?spm_id_from=333.999.
						list.card_archive.click&vd_source=16cf9fe495e6537559260c9d32da5153
*
************************************************************************/

#include "trace_line.h"
#include "camera.h"

#define BASIC_SPEED 400
#define Cir_SPEED 	100

//-------------------------------------------------------------------------------------------------------------------
//  @data 数据部分
//-------------------------------------------------------------------------------------------------------------------
//线程控制块
rt_thread_t 	trace_line_thread;
//信号量控制块
rt_sem_t 		trace_line_sem;
//原始图像
uint8 			frame[MT9V03X_W][MT9V03X_H];

//PID控制块
Pos_PID_t TraceLine_Yaw_Con;
Pos_PID_t TraceLine_Vx_Con;
Pos_PID_t TraceLine_Normal_Con;

float TraceLine_Aver_Offset;
int32 TraceLine_Last_Offset;
int32 TraceLine_Forward_V;

//基础速度
float speed_forward = 0;

uint16_t speed_decision_mark(){

	uint8_t lose_seq = 0;
	//统计缺线数量
	for(int i = imgRow-1;i>=0; i--){
		if(Image_S.leftBroder[i] == LEFT_LOSE_VALUE && Image_S.rightBroder[i] == RIGHT_LOSE_VALUE)
			lose_seq++;
		else
			TraceLine_Aver_Offset += (imgCol/2 - Image_S.MID_Table[i])*(i+1)*(i+1)/imgRow/imgRow;
	}

	if(lose_seq >= 0.3*imgRow)
		return 250;

	
}

/**
 * @brief 巡线策略函数
 * 
 */
extern uint8_t cir_speed_flag;
float quan;
void trace_line_method()
{
	uint8_t row_begin = 20;
	float mid_offset = 1.65;
	//速度控制权判定
	if(Car_Speed_ConRight == Con_By_TraceLine){
		//策略1 常规巡线
		//计算总偏差值
		uint16_t valid_line = 0;
		for(int i = imgRow-1; i>=row_begin;i--){
			if(Image_S.leftBroder[i] != LEFT_LOSE_VALUE || Image_S.rightBroder[i] != RIGHT_LOSE_VALUE){
				quan = (1-((float)(i+1)/(float)imgRow-1)*((float)(i+1)/(float)imgRow-1));
				TraceLine_Aver_Offset += (imgCol/2 - Image_S.MID_Table[i])*quan;
				//TraceLine_Aver_Offset += (imgCol/2 - Image_S.MID_Table[i])*(i+1)/imgRow*(i+1)/imgRow;
				valid_line+=1;
			}
		}
		if(valid_line != 0){
			TraceLine_Aver_Offset /= valid_line;
			// TraceLine_Aver_Offset -= valid_line*0.2;
		}
		else
			TraceLine_Aver_Offset = 0;


		//策略1 无中间环
		float yaw_now = Pos_PID_Controller(&TraceLine_Normal_Con,TraceLine_Aver_Offset - icm20602_gyro_z/100);
		float vx = Pos_PID_Controller(&TraceLine_Vx_Con,TraceLine_Aver_Offset);

		float speed_now = (Current_Road == CirculeRoads && cir_speed_flag)?Cir_SPEED:BASIC_SPEED;

		speed_now = speed_now<0? 0:speed_now;
		Car_Change_Speed(vx,speed_now,yaw_now);
		TraceLine_Aver_Offset = 0;

		//策略2 加上角度环
		// float yaw_now = Pos_PID_Controller(&TraceLine_Yaw_Con,TraceLine_Aver_Offset);
		// float vx = Pos_PID_Controller(&TraceLine_Vx_Con,TraceLine_Aver_Offset);

		// float speed_now = (Current_Road == CirculeRoads)?Cir_SPEED:BASIC_SPEED;
		// speed_now = speed_now<0? 0:speed_now;

		// Car_Change_Yaw(Att_CurrentYaw + yaw_now);
		// Car_Change_Speed(0,speed_now,0);
		// TraceLine_Aver_Offset = 0;

	}
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      巡线初始化函数
//  @param      void
//  @return     void
//  @e.g.       初始化了摄像头屏幕 以及注册了线程
//-------------------------------------------------------------------------------------------------------------------
void trace_line_init()
{
	rt_kprintf("trace line module init:\n");
	
	//初始化PID控制块 
	// Pos_PID_Init(&TraceLine_Yaw_Con,-10,0,-10);//八邻 时的参数

	Pos_PID_Init(&TraceLine_Yaw_Con,-2,0,0);
	TraceLine_Yaw_Con.Output_Max = 150;
	TraceLine_Yaw_Con.Output_Min = -150;
	TraceLine_Yaw_Con.Value_I = 200;
	TraceLine_Yaw_Con.Ref = 0;
	// 250 5
	Pos_PID_Init(&TraceLine_Vx_Con,5.5,0,1);
	TraceLine_Vx_Con.Output_Max = 200;
	TraceLine_Vx_Con.Output_Min = -200;
	TraceLine_Vx_Con.Value_I = 200;
	TraceLine_Vx_Con.Ref = 0;
	//250 7.2 10
	Pos_PID_Init(&TraceLine_Normal_Con,-8.0,0,-2);
	TraceLine_Normal_Con.Output_Max = 300;
	TraceLine_Normal_Con.Output_Min = -300;
	TraceLine_Normal_Con.Value_I = 200;
	TraceLine_Normal_Con.Ref = 0;

	
	//初始化信号量
	trace_line_sem = rt_sem_create("trace_line_sem",0,RT_IPC_FLAG_FIFO);
	if(trace_line_sem == RT_NULL){
		rt_kprintf("trace_line_sem created failed\n");
		while(1);
	}
	//初始化线程
	trace_line_thread = rt_thread_create("trace line",trace_line_entry,NULL,4096,2,1000);

	if(trace_line_thread != RT_NULL){
		rt_kprintf("trace line thread created successfully!\n");
		rt_thread_startup(trace_line_thread);
	}
	else 
		rt_kprintf("trace_line_thread created failed\n");

	// MCX_Change_Mode(MCX_Detection_Mode);
}


//-------------------------------------------------------------------------------------------------------------------
//  @brief      巡线线程运行函数
//  @param      void
//  @return     void
//  @e.g.       无
//-------------------------------------------------------------------------------------------------------------------
void trace_line_entry()
{
	static int tick = 0;
	static int push_flag = 0;
	while(1)
	{
		if(mt9v03x_finish_flag)
		{	
			Vision_Handle();
			trace_line_method();
			mt9v03x_finish_flag = 0;
		}
		rt_thread_delay(1);

		
		if(MCX_Detection_Flag){
			rt_kprintf("trace_line:get into push box task\n");
			Car_Change_Speed(0,0,0);
			rt_thread_delay(100);
			rt_sem_release(locate_box_sem);
			rt_sem_take(trace_line_sem,RT_WAITING_FOREVER);

			tick = rt_tick_get();
			push_flag = 1;
			rt_kprintf("trace_line:return to trace line task\n");
			MCX_Detection_Flag = 0;
		}
		
		if(push_flag && (rt_tick_get() - tick > 2000)){
			MCX_Change_Mode(MCX_Detection_Mode);
			push_flag = 0;
		}
	}
			
		
	
}
