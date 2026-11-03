#ifndef __BSP_SYSTEM_H
#define __BSP_SYSTEM_H

/* ========== C 语言头文件 ===================== */
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ================ TI 外设配置头文件 ================ */
#include "ti_msp_dl_config.h"

/* ================ 驱动层头文件 ===================== */
#include "uart_driver.h"        
#include "encoder_driver.h"     
#include "motor_driver.h"       
#include "button_driver.h"      
#include "pid.h"                
#include "Time.h"         
#include "OLED.h"
#include "red_gray_driver.h"				// 循迹：当前使用八路红外传感器，UART1 通道。 */
// #include "lf_04.h"
#include "imu_driver.h"
#include "icm42688_driver.h"



/* =================== 应用层头文件 ====================== */
#include "scheduler.h"
#include "uart_app.h"
#include "oled_app.h"
#include "gray_app.h"
#include "user_motor.h"
#include "user_pid.h"
#include "imu_app.h"
#include "imu_imu_app.h"





/* ============= 全局变量声明 ============= */
extern volatile uint32_t uwTick;
extern volatile float g_line_position_error;
extern volatile bool gray_noroad_enable;

#endif
