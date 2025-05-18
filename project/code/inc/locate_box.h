#ifndef __LOCATE_PICTURE_H__
#define __LOCATE_PICTURE_H__

#include "zf_common_headfile.h"

extern rt_sem_t locate_box_sem;
extern rt_thread_t locate_box_thread;

void locate_box_init();



#endif