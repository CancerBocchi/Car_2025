#include "locate_box.h"
#include "math.h"

#define AREA_CON_REF	120//160//高度控制期望
// #define X_CON_REF	
#define X_CON_REF		160

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
 * @brief 	定位测试函数
*/
void sorronding_test(){
	
	Car_Start();
	while(1){
		if(MCX_rx_flag){
			
			//Car_Change_Speed(0,Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));

			//Car_Change_Speed(Pos_PID_Controller(&locate_box_data.Transverse_pid,center_x),0,0);
			Car_Change_Speed(250,Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
			rt_kprintf("%d,%d\n",center_x,center_y);
			MCX_rx_flag = 0;
		}
		rt_thread_delay(1);
			
	}


}

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
	float sys_rate = Vision_CalSymRate(push_img_bw,188,120,(point_t){0,30},(point_t){25,119},1);
	float left_black_rate = Vision_CalBlackRate(push_img_bw,188,120,(point_t){0,30},(point_t){25,119});
	//float right_black_rate = Vision_CalBlackRate(push_img_bw,188,120,(point_t){187-20,0},(point_t){187,119});

	ips200_draw_line(20			,119+100		,20			,0+100		,RGB565_RED);
	ips200_draw_line(188-20		,119+100		,187-20		,0+100		,RGB565_RED);

	return (sys_rate > 0.82f && left_black_rate < 0.65f)?1:0;

}

/**
 * @brief 通过色块来判断是否应该推
 * 
 */
void test_RotateTo_Cor(){

	Car_Start();
	// Car_Rotate(0);//控制住车的方向

	while(1){
		uint8_t ShouldPush = 0;
		if(mt9v03x_finish_flag){

			Threshold = Camera_My_Adapt_Threshold(mt9v03x_image[0],IMAGE_COL,IMAGE_ROW);

			for(int i=0;i<MT9V03X_H;i++)
				for(int j=0;j<MT9V03X_W;j++)
					push_img_bw[i][j] = mt9v03x_image[i][j]>Threshold? 255:0;
				
			ips200_show_gray_image(0, 100, (const uint8 *)push_img_bw, 188, 120, 188, 120, 0);
			
			ShouldPush = PushBox_IsDirectionCorrect();
			rt_kprintf("%d\n",ShouldPush);
		}

		if(MCX_rx_flag){

			// if(!ShouldPush)
				Car_Change_Speed(200,Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
			// else
			// 	Car_Change_Speed(0,0,0);

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
			Vision_GetSegment(Image_S.leftBroder,1);
			Vision_GetSegment(Image_S.rightBroder,0);
			
			//获取中线
			for(int i=imgRow-1;i>=0;i--)
				Image_S.MID_Table[i]=(int16)((Image_S.rightBroder[i]+Image_S.leftBroder[i])/2);

			Vision_Draw();

			int n = 0;
			float k_Left = 0;
			float k_Right = 0;
			float k_Midd = 0;

			for(int i = F.segment_n_L;i>0;i--){
				if(!IsLose(F.my_segment_L[i-1])){
					k_Left += Line_GetAverK(Image_S.leftBroder,F.my_segment_L[i-1].begin,F.my_segment_L[i-1].end);	
					n++;
				}
			}
			k_Left = (n == 0)?0:k_Left/n;
			n = 0;

			for(int i = F.segment_n_R;i>0;i--){
				if(!IsLose(F.my_segment_R[i-1])){
					k_Right += Line_GetAverK(Image_S.rightBroder,F.my_segment_R[i-1].begin,F.my_segment_R[i-1].end);	
					n++;
				}
			}
			k_Right = (n==0)?0:k_Right/n;

			n = (k_Left == 0 || k_Right == 0)?1:2;

			float theta_offset = atan((k_Right+k_Left)/n)*180/PI;
			
			rt_kprintf("Locate_theta:%.2f\n",theta_offset+90);

			mt9v03x_finish_flag = 0;
		}
		rt_thread_delay(1);
		
	}


}

/**
 * @brief   使用中线斜率来空 
 * 
*/
void push_box(uint8_t l_or_r){
	// Car_DistanceMotion(0,60,1.5);
	// Car_DistanceMotion(0,-30,1.5);

	Car_Change_Speed(0,300,0);
	//确保在赛道上
	int tick = 0;
	while(tick <= 10)
		tick = (!gpio_get_level(C6))?tick + 1:0;

	tick = 0;				
	//确保不再赛道上
	while(tick <= 10)
		tick = (gpio_get_level(C6))?tick + 1:0;

	Car_Change_Speed(0,0,0);
	rt_thread_delay(200);
	Car_DistanceMotion(0,-30,0.7);
	Car_Rotate((l_or_r == PUSH_RIGHT)?90:-90);
	rt_thread_delay(250);
}

/**
 * @brief 矫正测试函数 先角度 然后角度+前后 然后转圈推 
 * 
 */
#define Angle_Correct_State 	0x01
#define Location_Correct_State	0x02
#define Push_Box_State			0x03

void direction_correction_test1(){

	Car_Start();
	Car_Speed_ConRight = Con_By_TraceLine;

	uint8_t angle_state = 0;
	float init_angle;
	uint8_t finish_flag = 0;
	uint8_t l_or_r;
	//abs(center_x - 127)<5
	while(!finish_flag){

		uint8_t ShouldPush = 0;
		//获取图像信息
		if(mt9v03x_finish_flag){
			Threshold = Camera_My_Adapt_Threshold(mt9v03x_image[0],IMAGE_COL,IMAGE_ROW);

			for(int i=0;i<MT9V03X_H;i++)
				for(int j=0;j<MT9V03X_W;j++)
					push_img_bw[i][j] = mt9v03x_image[i][j]>Threshold? 255:0;
				
			ips200_show_gray_image(0, 100, (const uint8 *)push_img_bw, 188, 120, 188, 120, 0);

			ShouldPush = PushBox_IsDirectionCorrect();
			rt_kprintf("%d\n",ShouldPush);
			

			mt9v03x_finish_flag = 0;
		}


		if(MCX_rx_flag){

			switch (angle_state)
			{
			case Angle_Correct_State:
				Car_Change_Speed(0,Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));//Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x)
				if(abs(center_x - X_CON_REF)<=10 && abs(center_y - AREA_CON_REF)<=15){
					Car_Change_Speed(0,0,0);
					rt_thread_delay(150);
					init_angle = Att_CurrentYaw;
					rt_kprintf("push box: init yaw is %.2f\n",init_angle);
					angle_state = Location_Correct_State;
					Art_DataClear();
					uint8_t tick = 0;
					int class; 
					//等待分类
					while(Art_GetData());
					class = Art_GetData();
					Art_DataClear();
					while(tick < 3){

						if(Art_GetData()){
							if(class == Art_GetData()){
								tick++;
							}
							else{
								class = Art_GetData();
								tick = 0;
							}
							if(tick < 3)
								Art_DataClear();
						}
					}
					l_or_r = Class_Add(Art_GetData());
					Art_DataClear();

				}

				break;

			case Location_Correct_State:
				Car_Change_Speed((l_or_r == PUSH_RIGHT)?-250:250,
									Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),
									Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
				if(/*fabs(Att_CurrentYaw - init_angle)>=85*/ShouldPush){
					angle_state = Push_Box_State;
					Car_Change_Speed(0,0,0);
					rt_thread_delay(150);
					Car_Rotate(0);
				}
				break;

			case Push_Box_State:
				Car_Change_Speed(Pos_PID_Controller(&locate_box_data.Transverse_pid,center_x),Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),0);
				if(abs(center_x - X_CON_REF)<=10){
					push_box(l_or_r);
					MCX_Change_Mode(MCX_Reset_Mode);
					Car_Speed_ConRight = Con_By_TraceLine;
					finish_flag = 1;
				}
				break;
			
			default:
				Car_Change_Speed(0,0,Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
				if(abs(center_x - X_CON_REF)<=20)
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
	
	direction_correction_test1();
	//test_RotateTo_Cor();
	//test_midK_Cor();
	//sorronding_test();
	
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
	
	//纵向PID初始化 1.5 0 0.5
	Pos_PID_Init(&locate_box_data.Longitudinal_pid,3,0,0.5);
	locate_box_data.Longitudinal_pid.Output_Max = 500;
	locate_box_data.Longitudinal_pid.Output_Min = -500;
	locate_box_data.Longitudinal_pid.Value_I_Max = 500;
	locate_box_data.Longitudinal_pid.Ref = AREA_CON_REF;
	
	//横向PID初始化 1.2
	Pos_PID_Init(&locate_box_data.Transverse_pid,1.3,0,0);
	locate_box_data.Transverse_pid.Output_Max = 500;
	locate_box_data.Transverse_pid.Output_Min = -500;
	locate_box_data.Transverse_pid.Value_I_Max = 500;
	locate_box_data.Transverse_pid.Ref = X_CON_REF;

	//一次矫正PID初始化 -1.4,0,-0.7
	Pos_PID_Init(&locate_box_data.Dir_Cen_pid,-1.0,0,0);
	locate_box_data.Dir_Cen_pid.Output_Max = 1000;
	locate_box_data.Dir_Cen_pid.Output_Min = -1000;
	locate_box_data.Dir_Cen_pid.Value_I_Max = 500;
	locate_box_data.Dir_Cen_pid.Ref = X_CON_REF;

	//二次矫正PID初始化
	Pos_PID_Init(&locate_box_data.Dir_pid,2,0,0);
	locate_box_data.Dir_pid.Output_Max = 1000;
	locate_box_data.Dir_pid.Output_Min = -1000;
	locate_box_data.Dir_pid.Value_I_Max = 500;
	locate_box_data.Dir_pid.Ref = 0;

}
