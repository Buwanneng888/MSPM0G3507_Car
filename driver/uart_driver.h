#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#include <stdint.h>

#include "ringbuffer.h"
#include "ti_msp_dl_config.h"

#define UART1_BUFFER_SIZE         256U
#define UART1_DMA_RX_BUFFER_SIZE  256U
#define UART2_BUFFER_SIZE         256U
#define UART2_DMA_RX_BUFFER_SIZE  256U
#define UART3_BUFFER_SIZE         256U
#define UART3_DMA_RX_BUFFER_SIZE  256U

void Uart_Tx_Init(void);

int my_printf(UART_Regs *uart, const char *format, ...);

void uart_send_char(UART_Regs *uart, char ch);
void uart_send_string(UART_Regs *uart, char *str, int length);

void Uart1_Rx_Init(void);
void Uart1_Rx_Update(void);
rt_size_t Uart1_Read(uint8_t *buffer, uint16_t length);
rt_size_t Uart1_Available(void);

void Uart2_Rx_Init(void);
void Uart2_Rx_Update(void);
rt_size_t Uart2_Read(uint8_t *buffer, uint16_t length);
rt_size_t Uart2_Available(void);

void Uart3_Rx_Init(void);
void Uart3_Rx_Update(void);
rt_size_t Uart3_Read(uint8_t *buffer, uint16_t length);
rt_size_t Uart3_Available(void);

#endif
