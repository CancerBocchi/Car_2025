#include "locate_box.h"

#define AREA_CON_REF	180//�����������
#define X_CON_REF		0//x�����������

rt_sem_t locate_box_sem;
rt_thread_t locate_box_thread;

struct{

	Pos_PID_t 	Transverse_pid;		//����pid
	Pos_PID_t 	Longitudinal_pid; 	//����pid
	Pos_PID_t 	Dir_Cen_pid;		//����ǿ���pid�����ڵ�һ��У׼
	Pos_PID_t  	Dir_pid;			//����ǿ���pid�����ڵڶ���У׼

	uint8_t 	locate_debug_flag;	//ץȡͼƬ��x����

}locate_box_data;


/**
 * @brief �ж������Ƿ����м�
 * 
 * @return uint8_t 1������ 0��������
 */
#define SIDE_WIDTH		(int)(20)
#define SIDE_HEIGHT		(int)(20)
#define TOP_WIDTH		(int)(60)
#define TOP_HEIGHT 		(int)(20)

#define SIDE_RATE_THD	0.05f
#define TOP_RATE_THD	0.95f

uint8_t PushBox_IsDirectionCorrect(void){
	float left_black_rate = Vision_CalBlackRate(my_image_BW, 0,69,0+SIDE_WIDTH,69-SIDE_HEIGHT);
	float right_black_rate = Vision_CalBlackRate(my_image_BW, 187,69,187-SIDE_WIDTH,69-SIDE_HEIGHT);
	float top_black_rate = Vision_CalBlackRate(my_image_BW, 93-TOP_WIDTH/2,TOP_HEIGHT,94+TOP_WIDTH/2,0);

	return (left_black_rate < SIDE_RATE_THD && 
				 right_black_rate < SIDE_RATE_THD && 
				 top_black_rate > TOP_RATE_THD)?1:0;
}

/**
 * @brief ͨ��ɫ�����ж��Ƿ�Ӧ����
 * 
 */
void test_RotateTo_Cor(){

	Car_Start();
	// Car_Rotate(0);//����ס���ķ���

	while(1){
		uint8_t ShouldPush = 0;
		if(mt9v03x_finish_flag){
			Camera_PreProcess();
			Camera_FindMyLine();
			
			//��ȡ����
			for(int i=imgRow-1;i>=0;i--)
				Image_S.MID_Table[i]=(int16)((Image_S.rightBroder[i]+Image_S.leftBroder[i])/2);

			Vision_Draw();
			
			ShouldPush = PushBox_IsDirectionCorrect();
		}

		if(MCX_rx_flag){

			if(ShouldPush)
				Car_Change_Speed(200,Pos_PID_Controller(&locate_box_data.Longitudinal_pid,center_y),Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));
			
			else
				Car_Change_Speed(0,0,0);

				

			MCX_rx_flag = 0;
		}
	}
}

/**
 * @brief   ʹ������б������
 * 
 */
void test_midK_Cor(){

	Car_Start();
	// Car_Rotate(0);//����ס���ķ���

	while(1){
		if(mt9v03x_finish_flag){
			Camera_PreProcess();
			Camera_FindMyLine();
			
			//��ȡ����
			for(int i=imgRow-1;i>=0;i--)
				Image_S.MID_Table[i]=(int16)((Image_S.rightBroder[i]+Image_S.leftBroder[i])/2);

			Vision_Draw();

			//��ȡ�������ߵ�б��ƽ��ֵ
			float aver_error;
			aver_error  = Line_GetAverK(Image_S.MID_Table,69,69-10);
			//����ƽ��б��Ϊ0
			Car_Change_Speed(Pos_PID_Controller(&locate_box_data.Dir_pid,aver_error),0,
							 Pos_PID_Controller(&locate_box_data.Dir_Cen_pid,center_x));

			mt9v03x_finish_flag = 0;
		}
		rt_thread_delay(1);
		
	}


}

/**
 * @brief �������Ժ���
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
				if(abs(center_x - X_CON_REF)<=4){
					Car_Change_Speed(0,0,0);
					rt_thread_delay(100);
					// Car_DistanceMotion(0,60,1.5);
					// Car_DistanceMotion(0,-30,1.5);

					Car_Change_Speed(0,200,0);
					while(!gpio_get_level(C6));
					while(gpio_get_level(C6));

					Car_Change_Speed(0,0,0);
					while(1);


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
 * @brief ���ڶ�λץȡͼƬdebug�ĳ���
 * 
 */
void locate_box_debug(){
	
	//direction_correction_test();
	
}

/**
 * @brief ��λͼƬ���к��� ������ʽ������
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
	
	//���Ա�־λ 0---������ 1---����
	locate_box_data.locate_debug_flag = 1;
	
	locate_box_sem = rt_sem_create("locate",0,RT_IPC_FLAG_FIFO);
	if(locate_box_sem == RT_NULL){
		rt_kprintf("locate_box_sem created failed\n");
		while(1);
	}

	locate_box_thread = rt_thread_create("locate",locate_box_entry,RT_NULL,4096,2,1000);
	rt_thread_startup(locate_box_thread);
	
	//����PID��ʼ��
	Pos_PID_Init(&locate_box_data.Longitudinal_pid,1.2,0,0.5);
	locate_box_data.Longitudinal_pid.Output_Max = 500;
	locate_box_data.Longitudinal_pid.Output_Min = -500;
	locate_box_data.Longitudinal_pid.Value_I_Max = 500;
	locate_box_data.Longitudinal_pid.Ref = AREA_CON_REF;
	
	//����PID��ʼ��
	Pos_PID_Init(&locate_box_data.Transverse_pid,1.2,0,0.5);
	locate_box_data.Transverse_pid.Output_Max = 500;
	locate_box_data.Transverse_pid.Output_Min = -500;
	locate_box_data.Transverse_pid.Value_I_Max = 500;
	locate_box_data.Transverse_pid.Ref = X_CON_REF;

	//һ�ν���PID��ʼ��-1.3,0,-0.5
	Pos_PID_Init(&locate_box_data.Dir_Cen_pid,-0.8,0,0);
	locate_box_data.Dir_Cen_pid.Output_Max = 500;
	locate_box_data.Dir_Cen_pid.Output_Min = -500;
	locate_box_data.Dir_Cen_pid.Value_I_Max = 500;
	locate_box_data.Dir_Cen_pid.Ref = X_CON_REF;

	//���ν���PID��ʼ��
	Pos_PID_Init(&locate_box_data.Dir_pid,2,0,0);
	locate_box_data.Dir_pid.Output_Max = 500;
	locate_box_data.Dir_pid.Output_Min = -500;
	locate_box_data.Dir_pid.Value_I_Max = 500;
	locate_box_data.Dir_pid.Ref = 0;

}
