/* Licence
 * Author:  Buwannengfeng
 * Version: V0.10
 * Time:    2026/10/06
 */

#include "bsp_system.h"

static void user_config(void);

int main(void)
{
    SYSCFG_DL_init();
    user_config();
    scheduler_init();


    while (1) 
		{
        scheduler_run();
    }
}

static void user_config(void)
{
#ifdef UART_0_INST_INT_IRQN
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
#endif

#ifdef UART_1_INST_INT_IRQN
    NVIC_ClearPendingIRQ(UART_1_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_1_INST_INT_IRQN);
#endif

#ifdef UART_2_INST_INT_IRQN
    NVIC_ClearPendingIRQ(UART_2_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_2_INST_INT_IRQN);
#endif

#ifdef UART_3_INST_INT_IRQN
    NVIC_ClearPendingIRQ(UART_3_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_3_INST_INT_IRQN);
#endif

#ifdef ENCODER_INT_IRQN
    NVIC_EnableIRQ(ENCODER_INT_IRQN);
#endif
}
