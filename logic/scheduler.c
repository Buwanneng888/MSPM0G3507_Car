/* Licence
 * Author:  Buwannengfeng
 * Version: V0.10
 * Time:    2026/10/06
 */

#include "scheduler.h"

volatile uint32_t uwTick = 0; 
uint8_t task_num = 0;        
static volatile uint8_t control_tasks_enabled = 0U; 


typedef struct {
    void (*task_func)(void);
    uint32_t rate_ms;       
    uint32_t last_run;      
} task_t;

/* 主循环任务表；每个任务依据 rate_ms 独立周期调度。 */
static task_t scheduler_task[] = {
   
//    {imu_imu_task,       10, 0},   
    {key_task,           10, 0},
    {gray_task,          10, 0},
    {oled_app_task,      40, 0},
    {Uart1_Task,         20, 0},   
//    {Uart2_Task,         20, 0},
//    {Uart3_Task,         20, 0},
		 /* PID在下面中断里自行打开*/
};

void System_Init(void)
{
    my_printf(UART_0_INST, "==== System Init ====\r\n");
    Uart_Init();
    oled_app_init();
    user_motor_init();
    encoder_config();
    PID_Init();
    gray_init();
    my_printf(UART_0_INST, "==== Finish ====\r\n");
}

void scheduler_init(void)
{
    /* 初始化期间关闭 10ms 控制任务，避免访问未初始化的结构体。 */
    control_tasks_enabled = 0U;

    System_Init();
    task_num = sizeof(scheduler_task) / sizeof(task_t);
	
    control_tasks_enabled = 1U;
}

void scheduler_run(void)
{
    for (uint8_t i = 0; i < task_num; i++)
		{
        uint32_t now_time = uwTick;

        if (now_time >= scheduler_task[i].rate_ms + scheduler_task[i].last_run)
				{
            scheduler_task[i].last_run = now_time;
            scheduler_task[i].task_func();
        }
    }
}

void SysTick_Handler(void)
{
    static uint8_t control_tick_div = 0U; 

    uwTick++;

    if (control_tasks_enabled != 0U) 
				{
						control_tick_div++;
						if (control_tick_div >= 10U) 
							{
								control_tick_div = 0U;
								encoder_task();
		//					imu_imu_task();									 /* ICM42688*/
		//          PID_Task();
							}
			}
		else 
			{
					control_tick_div = 0U;
			}
}

void DL_Delay(uint32_t delayMs)
{
    uint32_t startTick = uwTick;

    while ((uwTick - startTick) < delayMs) {
        ;
    }
}
