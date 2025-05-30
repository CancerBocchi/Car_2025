#include "locate_box.h"

#define AREA_CON_REF	180//面积控制期望
#define X_CON_REF		0//x坐标控制期望

rt_sem_t locate_box_sem;
rt_thread_t locate_box_thread;

uint8_t push_img_bw[MT9V03X_H][MT9V03X_W];

struct{

	Pos_PID_t 	Transverse_pid;		//横向pid
	Pos_PID_t 	Longitudinal_pid; 	//纵向pid
	Pos_PID_t 	Dir_Cen_pid;		//航向角控制pid，用于第一次校准
	Pos_PID_t  	Dir_pid;			//航向角控制pid，用于第二次校准

	uint8_t 	locate_debug_flag;	//抓取图片的x坐标

}locate_box_data;


/**
 * @brief 判断箱子是否在中间
 * 
 * @return uint8_t 1可以推 0不可以推
 */
#define SIDE_WIDTH		(int)(20)
#define SIDE_HEIGHT		(int)(20)
#define TOP_WIDTH		(int)(60)
#define TOP_HEIGHT 		(int)(10)


#define SIDE_RATE_THD	0.1f
#define TOP_RATE_THD	0.9f

uint8_t PushBox_IsDirectionCorrect(void){
	
	// float left_black_rate = Vision_CalBlackRate(push_img_bw, 0,119,0+SIDE_WIDTH,119-SIDE_HEIGHT);
	// float right_black_rate = Vision_CalBlackRate(push_img_bw, 187,119,187-SIDE_WIDTH,119-SIDE_HEIGHT);
	// float top_black_rate = Vision_CalBlackRate(push_img_bw, 93-TOP_WIDTH/2,TOP_HEIGHT,94+TOP_WIDTH/2,0);

	// return (left_black_rate < SIDE_RATE_THD && 
	// 			 right_black_rate < SIDE_RATE_THD && 
	// 			 top_black_rate > TOP_RATE_THD)?1:0;

	float sys_rate = Vision_CalSymRate(push_img_bw,188,120,(point_t){0,0},(point_t){20,120});
	float left_black_rate = Vision_CalBlackRate(push_img_bw,188,120,(point_t){0,0},(point_t){20,120});

	return (sys_rate > 0.9f && left_black_rate < 0.8f)?1:0;

}

/**
 * @brief 通过色块来判断是否应该推
 * 
 */
void test_RotateTo_Cor(){

	//Car_Start();
	// Car_Rotate(0);//控制住车的方向

	while(1){
		uint8_t ShouldPush = 0;
		if(mt9v03x_finish_flag){
			//Camera_PreProcess();
			// Camera_FindMyLine();
			
			// //获取中线
			// for(int i=imgRow-1;i>=0;i--)
			// 	Image_S.MID_Table[i]=(int16)((Image_S.rightBroder[i]+Image_S.leftBroder[i])/2);

			//Vision_Draw();

			Threshold = Camera_My_Adapt_Threshold(mt9v03x_image[0],IMAGE_COL,IMAGE_ROW);

			for(int i=0;i<MT9V03X_H;i++)
				for(int j=0;j<MT9V03X_W;j++)
					push_img_bw[i][j] = mt9v03x_image[i][j]>Threshold? 255:0;
				
			ips200_show_gray_image(0, 100, (const uint8 *)push_img_bw, 188, 120, 188, 120, 0);



			ips200_draw_line(0				,119+100				,SIDE_WIDTH			,119+100				,RGB565_RED);
			ips200_draw_line(SIDE_WIDTH		,119+100				,SIDE_WIDTH			,119+100-SIDE_WIDTH	,RGB565_RED);
			ips200_draw_line(SIDE_WIDTH		,119+100-SIDE_WIDTH		,0					,119+100-SIDE_WIDTH	,RGB565_RED);
			ips200_draw_line(0				,119+100-SIDE_WIDTH		,0					,119+100				,RGB565_RED);

			ips200_draw_line(187			,119+100				,187-SIDE_WIDTH		,119+100				,RGB565_RED);
			ips200_draw_line(187-SIDE_WIDTH	,119+100				,187-SIDE_WIDTH		,119+100-SIDE_WIDTH	,RGB565_RED);
			ips200_draw_line(187-SIDE_WIDTH	,119+100-SIDE_WIDTH		,187				,119+100-SIDE_WIDTH	,RGB565_RED);
			ips200_draw_line(187			,119+100-SIDE_WIDTH		,187				,119+100				,RGB565_RED);

			ips200_draw_line(93-TOP_WIDTH/2	,0+100					,94+TOP_WIDTH/2		,0+100				,RGB565_RED);
			ips200_draw_line(94+TOP_WIDTH/2	,0+100					,94+TOP_WIDTH/2		,TOP_HEIGHT+100		,RGB565_RED);
			ips200_draw_line(94+TOP_WIDTH/2	,TOP_HEIGHT+100			,93-TOP_WIDTH/2		,TOP_HEIGHT+100		,RGB565_RED);
			ips200_draw_line(93-TOP_WIDTH/2	,TOP_HEIGHT+100			,93-TOP_WIDTH/2		,0+100			,RGB565_RED);

			
			ShouldPush = PushBox_IsDirectionCorrect();
			rt_kprintf("%d\n",ShouldPush);
		}

		if(MCX_rx_flag){

			if(!ShouldPush)
				Car_Change_Speed(150,Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
			else
				Car_Change_Speed(0,0,0);

			MCX_rx_flag = 0;
		}
	}
}

/**
 * @brief   使用中线斜率来空
 * 
 */
void test_midK_Cor(){

	//Car_Start();
	// Car_Stop();
	// Car_Rotate(0);//控制住车的方向

	while(1){
		if(mt9v03x_finish_flag){
			Camera_PreProcess();
			Camera_FindMyLine();
			
			//获取中线
			for(int i=imgRow-1;i>=0;i--)
				Image_S.MID_Table[i]=(int16)((Image_S.rightBroder[i]+Image_S.leftBroder[i])/2);

			Vision_Draw();

			//截取部分中线的斜率平均值
			float aver_error;
			aver_error  = Line_GetAverK(Image_S.MID_Table,69,69-10)*20;
			//控制平均斜率为0
			Car_Change_Speed(Pos_PID_Controller(&locate_box_data.Dir_pid,aver_error),0,
							 Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
			// Car_Change_Speed(0,0,Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,aver_error));

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

		uint8_t ShouldPush = 0;
		if(mt9v03x_finish_flag){
			//Camera_PreProcess();
			// Camera_FindMyLine();
			
			// //获取中线
			// for(int i=imgRow-1;i>=0;i--)
			// 	Image_S.MID_Table[i]=(int16)((Image_S.rightBroder[i]+Image_S.leftBroder[i])/2);

			//Vision_Draw();

			Threshold = Camera_My_Adapt_Threshold(mt9v03x_image[0],IMAGE_COL,IMAGE_ROW);

			for(int i=0;i<MT9V03X_H;i++)
				for(int j=0;j<MT9V03X_W;j++)
					push_img_bw[i][j] = mt9v03x_image[i][j]>Threshold? 255:0;
				
			ips200_show_gray_image(0, 100, (const uint8 *)push_img_bw, 188, 120, 188, 120, 0);



			ips200_draw_line(0				,119+100				,SIDE_WIDTH			,119+100				,RGB565_RED);
			ips200_draw_line(SIDE_WIDTH		,119+100				,SIDE_WIDTH			,119+100-SIDE_WIDTH	,RGB565_RED);
			ips200_draw_line(SIDE_WIDTH		,119+100-SIDE_WIDTH		,0					,119+100-SIDE_WIDTH	,RGB565_RED);
			ips200_draw_line(0				,119+100-SIDE_WIDTH		,0					,119+100				,RGB565_RED);

			ips200_draw_line(187			,119+100				,187-SIDE_WIDTH		,119+100				,RGB565_RED);
			ips200_draw_line(187-SIDE_WIDTH	,119+100				,187-SIDE_WIDTH		,119+100-SIDE_WIDTH	,RGB565_RED);
			ips200_draw_line(187-SIDE_WIDTH	,119+100-SIDE_WIDTH		,187				,119+100-SIDE_WIDTH	,RGB565_RED);
			ips200_draw_line(187			,119+100-SIDE_WIDTH		,187				,119+100				,RGB565_RED);

			ips200_draw_line(93-TOP_WIDTH/2	,0+100					,94+TOP_WIDTH/2		,0+100				,RGB565_RED);
			ips200_draw_line(94+TOP_WIDTH/2	,0+100					,94+TOP_WIDTH/2		,TOP_HEIGHT+100		,RGB565_RED);
			ips200_draw_line(94+TOP_WIDTH/2	,TOP_HEIGHT+100			,93-TOP_WIDTH/2		,TOP_HEIGHT+100		,RGB565_RED);
			ips200_draw_line(93-TOP_WIDTH/2	,TOP_HEIGHT+100			,93-TOP_WIDTH/2		,0+100			,RGB565_RED);

			
			ShouldPush = PushBox_IsDirectionCorrect();
			rt_kprintf("%d\n",ShouldPush);
		}

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
				Car_Change_Speed(150,Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
				if(/*fabs(Att_CurrentYaw - init_angle)>=85*/ShouldPush){
					angle_state = Push_Box_State;
					Car_Change_Speed(0,0,0);
					rt_thread_delay(200);
					Car_Rotate(0);
				}
				break;

			case Push_Box_State:
				Car_Change_Speed(Pos_PID_Controller(&locate_box_data.Transverse_pid,center_x),Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),0);
				if(abs(center_x - X_CON_REF)<=4){
					Car_Change_Speed(0,0,0);
					rt_thread_delay(100);
					// Car_DistanceMotion(0,60,1.5);
					// Car_DistanceMotion(0,-30,1.5);

					Car_Change_Speed(0,200,0);
					//确保在赛道上
					int tick = 0;
					while(tick <= 10)
						tick = (!gpio_get_level(C6))?tick + 1:0;

					tick = 0;				
					//确保不再赛道上
					while(tick <= 10)
						tick = (gpio_get_level(C6))?tick + 1:0;

					Car_Change_Speed(0,0,0);

					Car_DistanceMotion(0,-30,1);

					Car_Change_Yaw(init_angle);
					rt_thread_delay(500);
					MCX_Change_Mode(MCX_Detection_Mode);
					Car_Speed_ConRight = Con_By_TraceLine;
					finish_flag = 1;

				}
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
	test_RotateTo_Cor();
	//test_midK_Cor();

	
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
	gpio_init(C6,GPI,0,GPI_PULL_DOWN);

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
