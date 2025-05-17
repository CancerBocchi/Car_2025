#include "final.h"

rt_sem_t final_sem;
rt_thread_t final_thread;

int final_flag = 0;
uint8_t final_debug_mode = 0;

/**
 * @brief 最终任务运行函数
 * 
 */
void final_entry(){
    while(1){
        
        
    }
}

/**
 * @brief  初始化最终任务
 * 
 */
void final_init(){

}