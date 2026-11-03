#if 0
/*
 * 原循迹模块应用层头文件保留区。
 * 2026-07-29 更换为 UART1 八路红外循迹模块后停用，原代码不删除。
 */
#ifndef __GRAY_APP_H
#define __GRAY_APP_H

#include <stdint.h>
#include "bsp_system.h"

/**
 * @brief 灰度初始化函数
 * 
 * @param none
 * 
 * @return none
 */
//void user_gray_init(void);
//void user_gray_task(void);
//uint8_t gray_get_digital(void);




//////////////////////////////////
/*no mcu code*/
//////////////////////////////////
void gray_init(void);
void gray_task(void);
void gray_calibrate(void);
uint8_t gray_get_digital(void);

extern volatile bool gray_noroad_enable;   /**< 丢线状态：true=所有有效通道均未检测到黑线。 */
extern volatile bool gray_noroad_enable;   /**< 旧代码中的重复声明，保留以避免改动原接口。 */
extern volatile bool gray_turn_leftangle;  /**< 旧转角逻辑方向状态：true=按左转方向处理。 */


#endif
#endif

/* 当前启用 UART1 八路串口循迹模块。 */
#define GRAY_CHANNEL_COUNT  8U
//#define GRAY_CHANNEL_MASK   0xFFU



#ifndef __GRAY_APP_H
#define __GRAY_APP_H

#include <stdbool.h>
#include <stdint.h>

void gray_init(void);
void gray_task(void);
void gray_calibrate(void);
float BlackLine_SetTurn(void);

/** 八路黑线位图：bit0=x1(最左)，bit7=x8(最右)，置 1 表示检测到黑线。 */
uint8_t gray_get_digital(void);

/** UART1 模块在线状态：最近 500ms 内收到有效数字量帧时返回 1。 */
uint8_t gray_is_online(void);

extern volatile float g_line_position_error; /**< 循迹横向误差：负值=线在左侧，正值=线在右侧。 */
extern volatile bool gray_noroad_enable;     /**< 丢线状态：true=未检测到任何黑线。 */
extern volatile bool gray_turn_leftangle;    /**< 兼容旧转角逻辑的方向状态量。 */

#endif
