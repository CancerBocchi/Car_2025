#include "button.h"

rt_thread_t button_thread;

button_t button1 = {
	.BUTTON_PIN = BUTTON1_PIN,
	.lock_flag = 0,
	.Denoi_tick = 0,
	.button_status = BUTTON_IDLE,
};

button_t button2 = {
	.BUTTON_PIN = BUTTON2_PIN,
	.lock_flag = 0,
	.Denoi_tick = 0,
	.button_status = BUTTON_IDLE,
};

void button_entry(){
	while(1)
	{
		
	}
}

// ��ʼ����ť
void button_init(){

	gpio_init(BUTTON1_PIN,GPI,0,GPI_PULL_UP);
	gpio_init(BUTTON2_PIN,GPI,0,GPI_PULL_UP);
	
	//��ʼ���߳�
	button_thread = rt_thread_create("button_thread",button_entry,NULL,1024,3,1000);

	if(button_thread != RT_NULL){
		rt_kprintf("button_thread created successfully!\n");
		rt_thread_startup(button_thread);
	}
	else 
		rt_kprintf("button_thread created failed\n");

}

/**
 * @brief ��ťɨ�躯�� ���°���������Ӧ
 * 
 * @param button �������ƿ�
 * @param tick ����ʱ��
 * @return uint8_t ��⵽����ʱ������1����֮����0
 */
uint8_t button_scan_pushdown(button_t* button, uint16_t tick){
	//���²���û������
	if(gpio_get_level(button->BUTTON_PIN) == PUSH_DOWN_LEVEL
		&& !button->lock_flag){
		button->lock_flag = 1;
		button->Denoi_tick = rt_tick_get();
		return 1;
	}
	//���� ���� 
	if(gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL && 
	   button->lock_flag && (rt_tick_get() - button->Denoi_tick) >= tick){
		button->lock_flag = 0;
	}

	return 0;
}

/**
 * @brief ɨ��̰��ͳ���
 * 
 * @param button �������ƿ�
 * @param long_tick ������ʶ
 * @param short_tick �̰���ʶ
 * @return uint8_t ����״̬
 */
uint8_t button_scan_longshort(button_t* button, uint16_t long_tick, uint16_t short_tick){
	
	switch(button->button_status){

		case BUTTON_IDLE:
			if(gpio_get_level(button->BUTTON_PIN) == PUSH_DOWN_LEVEL){
				button->button_status = BUTTON_FLIT;
				button->Denoi_tick = rt_tick_get();
			}
			break;

		//���������ڶ̰�ʱ����ɨ��
		case BUTTON_FLIT:
			if(rt_tick_get() - button->Denoi_tick >= short_tick)
				button->button_status = BUTTON_TICK;
			break;

		case BUTTON_TICK:
		//����ʱ�����ɿ�
			if(gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL){
				//���ڳ���ʱ�� ���볤������ģʽ
				if(rt_tick_get() - button->Denoi_tick >= long_tick)
					button->button_status = BUTTON_LONG;
				//С�ڳ���ʱ�䣬ֱ�ӷ��ض̰�
				else{
					button->button_status = BUTTON_IDLE;
					return BUTTON_SHORTPRESS;
				}
			}
			break;
		
		//��������ģʽ���������ɿ� �ŷ��ؾ�ֹ״̬
		case BUTTON_LONG:
			if(gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL)
				button->button_status = BUTTON_IDLE;
			break;

	}

	return 0;

}