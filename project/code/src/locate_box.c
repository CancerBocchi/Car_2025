#include "locate_box.h"

#define AREA_CON_REF	180//面积控制期望
#define X_CON_REF		0//x坐标控制期望

rt_sem_t locate_box_sem;
rt_thread_t locate_box_thread;

struct{

	Pos_PID_t 	Transverse_pid;		//横向pid
	Pos_PID_t 	Longitudinal_pid; 	//纵向pid
	Pos_PID_t 	Dir_Cen_pid;		//航向角控制pid，用于第一次校准
	Pos_PID_t  	Dir_pid;			//航向角控制pid，用于第二次校准

	uint8_t 	locate_debug_flag;	//抓取图片的x坐标

}locate_box_data;

/**
 * @brief 使用扫线进行校准
 * 
 */
void locate_test(){

	Car_Start();
	// Car_Rotate(0);//控制住车的方向

	while(1){
		if(mt9v03x_finish_flag){
			Camera_PreProcess();
			Camera_FindMidLine();
			
			Vision_Draw();

			//获取中线
			for(int i=imgRow-1;i>=0;i--)
				Image_S.MID_Table[i]=(int16)((Image_S.rightBroder[i]+Image_S.leftBroder[i])/2);

			int16 MID_Table[imgRow];
			int16 leftBroder[imgRow];//左边边界
			int16 rightBroder[imgRow];//右边边界
			for(int i=imgRow-1;i>=0;i--){
				MID_Table[i] = Image_S.MID_Table[i];
				leftBroder[i] = Image_S.leftBroder[i];
				rightBroder[i] = Image_S.rightBroder[i];
			}

			for(int i=imgRow-1;i>0;i--)
			{
				if(MID_Table[i]>=188)
					MID_Table[i] = 187;
					MID_Table[i] = MID_Table[i]<0? 0:MID_Table[i];

				if(leftBroder[i]>=188)
					leftBroder[i] = 187;
					leftBroder[i] = leftBroder[i]<0? 0:leftBroder[i];

				if(rightBroder[i]>=188)
					rightBroder[i] = 187;
					rightBroder[i] = rightBroder[i]<0? 0:rightBroder[i]; 
							
				ips200_draw_point(MID_Table[i], i, RGB565_RED);
				ips200_draw_point(leftBroder[i], i, RGB565_BLUE);
				ips200_draw_point(rightBroder[i], i, RGB565_BROWN);
				//中线
				ips200_draw_point((int)(94), i, RGB565_GREEN);
			}



			//截取部分中线的平均值?
			float aver_error;
			float error;
			for(int i = 69-10; i<=69;i++)
				error = error + (imgCol/2 - Image_S.MID_Table[i])/10.0f;

			//控制环路
			aver_error = error;
			Car_Change_Speed(Pos_PID_Controller(&locate_box_data.Dir_pid,aver_error),0,Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
			error = 0;

			mt9v03x_finish_flag = 0;
		}
		rt_thread_delay(1);
		
	}

}

/**
 * @brief 矫正测试函数
 * 
 */
#define Angle_Correct_State 	0x01
#define Location_Correct_State	0x02
#define Push_Box_State			0x03

void direction_correction_test(){

	Car_Start();
	Car_Speed_ConRight = Con_By_TraceLine;

	uint8_t angle_state = 0;
	float init_angle;
	uint8_t finish_flag = 0;
	//abs(center_x - 127)<5
	while(!finish_flag){
		if(MCX_rx_flag){

			switch (angle_state)
			{
			case Angle_Correct_State:
				Car_Change_Speed(0,Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));//Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x)
				if(abs(center_x - X_CON_REF)<=5 && abs(center_y - AREA_CON_REF)<=10){
					Car_Change_Speed(0,0,0);
					rt_thread_delay(200);
					init_angle = Att_CurrentYaw;
					rt_kprintf("push box: init yaw is %.2f\n",init_angle);
					angle_state = Location_Correct_State;
				}

				break;

			case Location_Correct_State:
				Car_Change_Speed(200,Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
				if(fabs(Att_CurrentYaw - init_angle)>=90){
					angle_state = Push_Box_State;
					Car_Change_Speed(0,0,0);
					rt_thread_delay(200);
					Car_Rotate(0);
				}
				break;

			case Push_Box_State:
				Car_Change_Speed(Pos_PID_Controller(&locate_box_data.Transverse_pid,center_x),Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),0);
				// if(abs(center_x - X_CON_REF)<=4){
				// 	Car_Change_Speed(0,0,0);
				// 	rt_thread_delay(100);
				// 	Car_DistanceMotion(0,60,1.5);
				// 	Car_DistanceMotion(0,-30,1.5);

				// 	Car_Change_Yaw(init_angle);
				// 	rt_thread_delay(500);
				// 	MCX_Change_Mode(MCX_Detection_Mode);
				// 	Car_Speed_ConRight = Con_By_TraceLine;
				// 	finish_flag = 1;

				// }
				break;
			
			default:
				Car_Change_Speed(0,0,Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
				if(abs(center_x - X_CON_REF)<=10)
					angle_state = Angle_Correct_State;
				break;
			}
			
			MCX_rx_flag = 0;
		}
		rt_thread_delay(1);
	}



}

/**
 * @brief 用于定位抓取图片debug的程序
 * 
 */
void locate_box_debug(){
	
	//direction_correction_test();
	locate_test();
	
}

/**
 * @brief 定位图片运行函数 用于正式做任务
 * 
 */
void locate_box_pull(){


		
}

void locate_box_entry()
{
	while(1){
		rt_kprintf("push box:get into push task\n");
		rt_sem_take(locate_box_sem,RT_WAITING_FOREVER);

		(locate_box_data.locate_debug_flag == 1)?locate_box_debug():locate_box_pull();

		rt_kprintf("push box:return to the trace line task\n");
		rt_sem_release(trace_line_sem);
	}
}


void locate_box_init()
{
	rt_kprintf("locate_pic task init\n");
	
	//调试标志位 0---不调试 1---调试
	locate_box_data.locate_debug_flag = 1;
	
	locate_box_sem = rt_sem_create("locate",0,RT_IPC_FLAG_FIFO);
	if(locate_box_sem == RT_NULL){
		rt_kprintf("locate_box_sem created failed\n");
		while(1);
	}

	locate_box_thread = rt_thread_create("locate",locate_box_entry,RT_NULL,4096,2,1000);
	rt_thread_startup(locate_box_thread);
	
	//纵向PID初始化
	Pos_PID_Init(&locate_box_data.Longitudinal_pid,1.2,0,0.5);
	locate_box_data.Longitudinal_pid.Output_Max = 500;
	locate_box_data.Longitudinal_pid.Output_Min = -500;
	locate_box_data.Longitudinal_pid.Value_I_Max = 500;
	locate_box_data.Longitudinal_pid.Ref = AREA_CON_REF;
	
	//横向PID初始化
	Pos_PID_Init(&locate_box_data.Transverse_pid,1.2,0,0.5);
	locate_box_data.Transverse_pid.Output_Max = 500;
	locate_box_data.Transverse_pid.Output_Min = -500;
	locate_box_data.Transverse_pid.Value_I_Max = 500;
	locate_box_data.Transverse_pid.Ref = X_CON_REF;

	//一次矫正PID初始化-1.3,0,-0.5
	Pos_PID_Init(&locate_box_data.Dir_Cen_pid,-0.8,0,0);
	locate_box_data.Dir_Cen_pid.Output_Max = 500;
	locate_box_data.Dir_Cen_pid.Output_Min = -500;
	locate_box_data.Dir_Cen_pid.Value_I_Max = 500;
	locate_box_data.Dir_Cen_pid.Ref = X_CON_REF;

	//二次矫正PID初始化
	Pos_PID_Init(&locate_box_data.Dir_pid,2,0,0);
	locate_box_data.Dir_pid.Output_Max = 500;
	locate_box_data.Dir_pid.Output_Min = -500;
	locate_box_data.Dir_pid.Value_I_Max = 500;
	locate_box_data.Dir_pid.Ref = 0;

}
