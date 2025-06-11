/***********************************************************************
* һ����·����ƽ���ٶȣ�������Ϊ�����е���ͼ�����ߵ�ƫ������ƽ��ֵ
* ��һ����·���ƺ���ǣ�������Ϊ��һ����(LAST_OFFSET_POINT_NUM)���е�����ƽ��ֵ
* �ο����ӣ�https://www.bilibili.com/video/BV1ZT4y1D716/?spm_id_from=333.999.
						list.card_archive.click&vd_source=16cf9fe495e6537559260c9d32da5153
*
************************************************************************/

#include "trace_line.h"
#include "camera.h"

//-------------------------------------------------------------------------------------------------------------------
//  @data ���ݲ���
//-------------------------------------------------------------------------------------------------------------------
//�߳̿��ƿ�
rt_thread_t 	trace_line_thread;
//�ź������ƿ�
rt_sem_t 		trace_line_sem;
//ԭʼͼ��
uint8 			frame[MT9V03X_W][MT9V03X_H];

//PID���ƿ�
Pos_PID_t TraceLine_Yaw_Con;
Pos_PID_t TraceLine_Vx_Con;
Pos_PID_t TraceLine_Normal_Con;

int32 TraceLine_Aver_Offset;
int32 TraceLine_Last_Offset;
int32 TraceLine_Forward_V;

//�����ٶ�
float speed_forward = 0;

uint16_t speed_decision_mark(){

	uint8_t lose_seq = 0;
	//ͳ��ȱ������
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
 * @brief Ѳ�߲��Ժ���
 * 
 */
void trace_line_method()
{
	uint8_t row_begin = 20;
	float mid_offset=1.65;
	//�ٶȿ���Ȩ�ж�
	if(Car_Speed_ConRight == Con_By_TraceLine){
		//����1 ����Ѳ��
		//������ƫ��ֵ
		for(int i = imgRow-1; i>=row_begin;i--)
			TraceLine_Aver_Offset += (imgCol/2 - Image_S.MID_Table[i])*(i+1)*(i+1)/imgRow/imgRow;

		TraceLine_Aver_Offset /= imgRow;

		float yaw_now = Pos_PID_Controller(&TraceLine_Normal_Con,TraceLine_Aver_Offset);
		float vx = Pos_PID_Controller(&TraceLine_Vx_Con,TraceLine_Aver_Offset);


		float speed_now = speed_forward-1.5f*abs(TraceLine_Aver_Offset)*abs(TraceLine_Aver_Offset);
		speed_now = speed_now<0? 0:speed_now;

		Car_Change_Speed(vx,speed_now,yaw_now);

		TraceLine_Aver_Offset = 0;	


		//����2 ����߷�
		// float vx = Pos_PID_Controller(&TraceLine_Vx_Con,Center);
		// float yaw = Pos_PID_Controller(&TraceLine_Yaw_Con,Center);
		// Car_Change_Speed(vx,speed_forward,yaw);

		
	}
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      Ѳ�߳�ʼ������
//  @param      void
//  @return     void
//  @e.g.       ��ʼ��������ͷ��Ļ �Լ�ע�����߳�
//-------------------------------------------------------------------------------------------------------------------
void trace_line_init()
{
	rt_kprintf("trace line module init:\n");
	
	//��ʼ��PID���ƿ� 
	// Pos_PID_Init(&TraceLine_Yaw_Con,-10,0,-10);//���� ʱ�Ĳ���

	Pos_PID_Init(&TraceLine_Yaw_Con,3,1,0);
	TraceLine_Yaw_Con.Output_Max = 150;
	TraceLine_Yaw_Con.Output_Min = -150;
	TraceLine_Yaw_Con.Value_I = 200;
	TraceLine_Yaw_Con.Ref = 96;
	
	Pos_PID_Init(&TraceLine_Vx_Con,10,0,0);
	TraceLine_Vx_Con.Output_Max = 300;
	TraceLine_Vx_Con.Output_Min = -300;
	TraceLine_Vx_Con.Value_I = 200;
	TraceLine_Vx_Con.Ref = 0;

	Pos_PID_Init(&TraceLine_Normal_Con,-7.8,0,-10);
	TraceLine_Normal_Con.Output_Max = 200;
	TraceLine_Normal_Con.Output_Min = -200;
	TraceLine_Normal_Con.Value_I = 200;
	TraceLine_Normal_Con.Ref = 0;

	
	//��ʼ���ź���
	trace_line_sem = rt_sem_create("trace_line_sem",0,RT_IPC_FLAG_FIFO);
	if(trace_line_sem == RT_NULL){
		rt_kprintf("trace_line_sem created failed\n");
		while(1);
	}
	//��ʼ���߳�
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
//  @brief      Ѳ���߳����к���
//  @param      void
//  @return     void
//  @e.g.       ��
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
			rt_thread_delay(500);
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
