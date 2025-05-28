#include "button.h"

rt_thread_t button_thread;

void button_entry(){
	while(1)
	{
		
	}
}

// 初始化按钮
void button_init(){
	

	//初始化线程
	button_thread = rt_thread_create("button_thread",button_entry,NULL,1024,3,1000);

	if(button_thread != RT_NULL){
		rt_kprintf("button_thread created successfully!\n");
		rt_thread_startup(trace_line_thread);
	}
	else 
		rt_kprintf("button_thread created failed\n");

}

uint8_t button_scan_pushdown(button_t* button, uint16_t tick){
	//按下并且没有锁定
	if(gpio_get_level(button->BUTTON_PIN) == PUSH_DOWN_LEVEL
		&& !button->lock_flag){
		button->lock_flag = 1;
		button->Denoi_tick = rt_tick_get();
		return 1;
	}
	//锁定 并且 
	if(gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL && 
	   button->lock_flag && (rt_tick_get() - button->Denoi_tick) >= tick){
		button->lock_flag = 0;
	}

	return 0;
}


uint8_t button_scan_release(button_t* button, uint16_t tick){
	//按下并且没有锁定
	if(gpio_get_level(button->BUTTON_PIN) == PUSH_DOWN_LEVEL
		&& !button->lock_flag){
		button->lock_flag = 1;
		button->Denoi_tick = rt_tick_get();
	}
	//锁定 并且 
	if(gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL && 
	   button->lock_flag && (rt_tick_get() - button->Denoi_tick) >= tick){
		button->lock_flag = 0;
		return 1;
	}

	return 0;
}

uint8_t button_scan_longpress(button_t* button, uint16_t tick){
	//按下并且没有锁定
	if(gpio_get_level(button->BUTTON_PIN) == PUSH_DOWN_LEVEL
		&& !button->lock_flag){
		button->lock_flag = 1;
		button->Denoi_tick = rt_tick_get();
	}
	//锁定 并且 达到长按时间
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
 * @brief 扫描短按和长按
 * 
 * @param button 按键控制块
 * @param long_tick 长按标识
 * @param short_tick 短按标识
 * @return uint8_t 按键状态
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