#include "zf_common_headfile.h"

rt_thread_t led_thread;

int Start_Flag = 0;

int exposure_time = 256;
extern uint8_t Class_Number; //分类个数

void led_thread_entry()
{
	gpio_init(B9,GPO,0, GPO_PUSH_PULL);
	while(1)
	{
		gpio_toggle_level(B9);
		rt_thread_delay(100);
	}
}

int main()
{

	led_thread = rt_thread_create("blink",led_thread_entry,NULL,1024,10,1000);
	rt_thread_startup(led_thread);

	debug_tool_init();
	
	gpio_init(C3,GPO,0,GPO_PUSH_PULL);
	
	
	rt_kprintf("\nSystem_Init:\n");

	rt_kprintf("----------  Basic Hardware Init ----------\n");
	
	car_motion_Init();
	buzzer_init();
	Camera_and_Screen_Init();
	MCX_UART_Init();
	Art_UART_Init();
	
	rt_kprintf("---------- task init ----------\n");
	locate_box_init();
	trace_line_init();


	rt_kprintf("--------- init end ----------\n");

	gpio_init(D16,GPI,0,GPI_PULL_UP);
	gpio_init(D17,GPI,0,GPI_PULL_UP);

	rt_thread_delay(1000);
	int l_or_r;

	while(1){

		if(!gpio_get_level(D16)){
			Car_Start();
			rt_thread_delay(2000);
			speed_forward = 300;
			// Car_Speed_ConRight = Con_By_AngleLoop;
			// rt_thread_delay(600);
			// while(Art_GetData() == 0);
			// Class_Add(Art_GetData());
			// Art_DataClear();
		}
		// if(speed_forward){
		// 	Car_Rotate(90);
		// 	rt_thread_delay(1000);
		// }


		rt_thread_delay(1);
	}

}
