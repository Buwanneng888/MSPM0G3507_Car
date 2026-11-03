#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "bsp_system.h"

/**
 * @brief 系统初始化：集中调用各模块 Init 并串口打点
 * 
 * @param none
 * 
 * @return none
 */
void System_Init(void);
/**
 * @brief 调度器初始化
 * 
 * @param none
 * 
 * @return none
 */
void scheduler_init(void);
/**
 * @brief 调度器运行函数
 * 
 * @param none
 * 
 * @return none
 */
void scheduler_run(void);
/**
 * @brief DL库延时函数(ms)
 * @param none
 * @return none
 */
void DL_Delay(uint32_t delayMs);
#endif