#ifndef __OPEN_ART_H__
#define __OPEN_ART_H__

#include "zf_common_headfile.h"

#define BUFFER_HEAD 0xAA
#define BUFFER_TAIL 0xFF
#define MCX_Detection_Mode  ('D')
#define MCX_Reset_Mode      ('R')


#define MCX_UART 			UART_3
#define MCX_UART_RX		UART3_RX_B23
#define MCX_UART_TX		UART3_TX_B22
#define MCX_UART_IRQn	LPUART3_IRQn

#define LEFT_PIC            0
#define RIGHT_PIC           1

extern uint8_t MCX_Detection_Flag;
extern int16_t center_x;
extern int16_t center_y;
extern uint8_t cur_PicNum;
extern uint8_t MCX_rx_flag;

void MCX_Change_Mode(uint8_t mode);
void MCX_UART_Init(void);
void MCX_uart_handle();
void MCX_Clear();

#endif


