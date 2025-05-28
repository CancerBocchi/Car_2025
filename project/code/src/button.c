#include "button.h"

rt_thread_t button_thread;

void button_entry(){
	while(1)
	{
		
	}
}

// ��ʼ����ť
void button_init(){
	

	//��ʼ���߳�
	button_thread = rt_thread_create("button_thread",button_entry,NULL,1024,3,1000);

	if(button_thread != RT_NULL){
		rt_kprintf("button_thread created successfully!\n");
		rt_thread_startup(trace_line_thread);
	}
	else 
		rt_kprintf("button_thread created failed\n");

}

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


uint8_t button_scan_release(button_t* button, uint16_t tick){
	//���²���û������
	if(gpio_get_level(button->BUTTON_PIN) == PUSH_DOWN_LEVEL
		&& !button->lock_flag){
		button->lock_flag = 1;
		button->Denoi_tick = rt_tick_get();
	}
	//���� ���� 
	if(gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL && 
	   button->lock_flag && (rt_tick_get() - button->Denoi_tick) >= tick){
		button->lock_flag = 0;
		return 1;
	}

	return 0;
}

uint8_t button_scan_longpress(button_t* button, uint16_t tick){
	//���²���û������
	if(gpio_get_level(button->BUTTON_PIN) == PUSH_DOWN_LEVEL
		&& !button->lock_flag){
		button->lock_flag = 1;
		button->Denoi_tick = rt_tick_get();
	}
	//���� ���� �ﵽ����ʱ��
	if(rt_tick_get() - button->Denoi_tick >= tick
		&& button->lock_flag && !button->long_press_flag){
		button->long_press_flag = 1;
		return 1;
	}

	if(button->long_press_flag && gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL){
		button->long_press_flag = 0;
		button->lock_flag = 0;
	}

	return 0;
}



#define BUTTON_IDLE		0x00
#define BUTTON_TICK		0x01
#define BUTTON_LONG		0x02
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
				button->button_status = BUTTON_TICK;
				button->Denoi_tick = rt_tick_get();
			}

			break;
		case BUTTON_TICK:
			if(rt_tick_get() - button->Denoi_tick >= long_tick){
				button->button_status = BUTTON_IDLE;
				return BUTTON_LONG;
			}
			else if(rt_tick_get() - button->Denoi_tick >= short_tick && 
				gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL){
				button->button_status = BUTTON_IDLE;
				return BUTTON_SHORTPRESS;
			}
			break;

		case BUTTON_LONG:
			if(gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL)
				button->button_status = BUTTON_IDLE;
			break;

	}

	return 0;

}