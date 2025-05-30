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

// 初始化按钮
void button_init(){

	gpio_init(BUTTON1_PIN,GPI,0,GPI_PULL_UP);
	gpio_init(BUTTON2_PIN,GPI,0,GPI_PULL_UP);
	
	//初始化线程
	button_thread = rt_thread_create("button_thread",button_entry,NULL,1024,3,1000);

	if(button_thread != RT_NULL){
		rt_kprintf("button_thread created successfully!\n");
		rt_thread_startup(button_thread);
	}
	else 
		rt_kprintf("button_thread created failed\n");

}

/**
 * @brief 按钮扫描函数 按下按键立即响应
 * 
 * @param button 按键控制块
 * @param tick 消抖时间
 * @return uint8_t 检测到按下时，返回1，反之返回0
 */
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
				button->button_status = BUTTON_FLIT;
				button->Denoi_tick = rt_tick_get();
			}
			break;

		//消抖，大于短按时间再扫描
		case BUTTON_FLIT:
			if(rt_tick_get() - button->Denoi_tick >= short_tick)
				button->button_status = BUTTON_TICK;
			break;

		case BUTTON_TICK:
		//若此时按键松开
			if(gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL){
				//大于长按时间 进入长按锁定模式
				if(rt_tick_get() - button->Denoi_tick >= long_tick)
					button->button_status = BUTTON_LONG;
				//小于长按时间，直接返回短按
				else{
					button->button_status = BUTTON_IDLE;
					return BUTTON_SHORTPRESS;
				}
			}
			break;
		
		//长按锁定模式，长按后松开 才返回静止状态
		case BUTTON_LONG:
			if(gpio_get_level(button->BUTTON_PIN) == !PUSH_DOWN_LEVEL)
				button->button_status = BUTTON_IDLE;
			break;

	}

	return 0;

}