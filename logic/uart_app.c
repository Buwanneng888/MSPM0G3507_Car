/* Licence
 * Author:  Buwannengfeng
 * Version: V0.10
 * Time:    2026/10/06
 */

#include "bsp_system.h"

static uint8_t uart1_data_buffer[UART1_BUFFER_SIZE + 1U];
static uint8_t uart2_data_buffer[UART2_BUFFER_SIZE + 1U];
static uint8_t uart3_data_buffer[UART3_BUFFER_SIZE + 1U];

void Uart_Init(void)
{
    my_printf(UART_0_INST, "uart init ok\r\n");
    Uart_Tx_Init();
    Uart1_Rx_Init();
    Uart2_Rx_Init();
    Uart3_Rx_Init();
}

void Uart1_Task(void)
{
    rt_size_t uart_data_len;


    Uart1_Rx_Update();
    uart_data_len = Uart1_Available();
    if (uart_data_len == 0U) {
        return;
    }

    if (uart_data_len > UART1_BUFFER_SIZE) {
        uart_data_len = UART1_BUFFER_SIZE;
    }

    uart_data_len = Uart1_Read(uart1_data_buffer, (uint16_t) uart_data_len);
    if (uart_data_len == 0U) {
        return;
    }

    uart1_data_buffer[uart_data_len] = '\0';

    /* 在这里接 UART1 的业务解析，调试打印只作为当前验证手段。 */
    my_printf(DEBUG_UART, "UART1 Ringbuffer:%s\r\n", uart1_data_buffer);

    memset(uart1_data_buffer, 0, uart_data_len + 1U);
}

void Uart2_Task(void)
{
    rt_size_t uart_data_len;

  
    Uart2_Rx_Update();
    uart_data_len = Uart2_Available();
    if (uart_data_len == 0U) {
        return;
    }

    if (uart_data_len > UART2_BUFFER_SIZE) {
        uart_data_len = UART2_BUFFER_SIZE;
    }

    uart_data_len = Uart2_Read(uart2_data_buffer, (uint16_t) uart_data_len);
    if (uart_data_len == 0U) {
        return;
    }

    uart2_data_buffer[uart_data_len] = '\0';

    /* 在这里接 UART2 的业务解析，调试打印只作为当前验证手段。 */
    my_printf(DEBUG_UART, "UART2 Ringbuffer:%s\r\n", uart2_data_buffer);

    memset(uart2_data_buffer, 0, uart_data_len + 1U);
}

void Uart3_Task(void)
{
    rt_size_t uart_data_len;


    Uart3_Rx_Update();
    uart_data_len = Uart3_Available();
    if (uart_data_len == 0U) {
        return;
    }

    if (uart_data_len > UART3_BUFFER_SIZE) {
        uart_data_len = UART3_BUFFER_SIZE;
    }

    uart_data_len = Uart3_Read(uart3_data_buffer, (uint16_t) uart_data_len);
    if (uart_data_len == 0U) {
        return;
    }

    uart3_data_buffer[uart_data_len] = '\0';

    /* 在这里接 UART3 的业务解析，调试打印只作为当前验证手段。 */
    my_printf(DEBUG_UART, "UART3 Ringbuffer:%s\r\n", uart3_data_buffer);

    memset(uart3_data_buffer, 0, uart_data_len + 1U);
}
