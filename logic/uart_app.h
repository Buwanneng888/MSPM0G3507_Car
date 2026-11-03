#ifndef __UART_APP_H__
#define __UART_APP_H__

#include "uart_driver.h"

#define DEBUG_UART UART_0_INST

void Uart_Init(void);
void Uart1_Task(void);
void Uart2_Task(void);
void Uart3_Task(void);

#endif
