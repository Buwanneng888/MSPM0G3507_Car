#ifndef __MOTOR_DRIVER_H
#define __MOTOR_DRIVER_H


#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_system.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 电机状态枚举
 */
typedef enum {
    MOTOR_STATE_STOP = 0,     // 停止
    MOTOR_STATE_FORWARD,      // 正转
    MOTOR_STATE_BACKWARD,     // 反转
    MOTOR_STATE_ERROR         // 错误
} MotorState_t;

/**
 * @brief 电机硬件配置结构体
 */
typedef struct {
    GPTIMER_Regs* htim;        // PWM定时器句柄
    DL_TIMER_CC_INDEX channel;   // PWM通道
    GPIO_Regs* dir_port;         // 方向控制GPIO端口
    uint32_t dir_pin;               // 方向控制GPIO引脚
		GPIO_Regs *dir_port1;
    uint32_t dir_pin1;
}MotorHW_t;

/**
 * @brief 电机驱动实体结构体
 */
typedef struct {
    MotorHW_t hw;                   // 硬件配置
    float speed;                   // 当前速度 (-100 到 +100)
    MotorState_t state;             // 当前状态
    uint8_t enable;                 // 使能标志
    uint8_t reverse;                // 电机安装方向 (0-正装, 1-反装)
} Motor_t;


extern Motor_t left_motor;
extern Motor_t right_motor;
/* Exported constants --------------------------------------------------------*/
#define MOTOR_SPEED_MAX         100.0f     // 最大速度
#define MOTOR_SPEED_MIN         -100.0f    // 最小速度
#define MOTOR_PWM_PERIOD        999      // PWM周期 (ARR值，与TIM1->ARR一致)
#define MOTOR_MIN_PWM_THRESHOLD 10      // 最小PWM阈值
#define MOTOR_PRECISION_SCALE   10.0f       // 精度缩放因子
#define MOTOR_MAX_PRECISION     1000        // 最大精度值
/* Exported macros -----------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/


/**
 * @brief 创建电机实体
 * @param motor: 电机实体指针
 * @param htim: PWM定时器句柄
 * @param channel: PWM通道 (TIM_CHANNEL_1~4)
 * @param dir_port: 方向控制GPIO端口
 * @param dir_pin: 方向控制GPIO引脚
 * @param reverse: 电机安装方向 (0-正装, 1-反装)
 * @retval 0: 成功, -1: 参数错误
 */
int8_t Motor_Create(Motor_t *motor,
                    GPTIMER_Regs *htim,
                    DL_TIMER_CC_INDEX channel,
                    GPIO_Regs *dir_port,
                    uint32_t dir_pin,
										GPIO_Regs *dir_port1,
                    uint32_t dir_pin1,
                    uint8_t reverse);

/**
 * @brief 设置电机速度
 * @param motor: 电机实体指针
 * @param speed: 速度值 (-100 到 +100)
 *               正数为正转，负数为反转，0为停止
 * @retval 0: 成功, -1: 参数错误
 */
int8_t Motor_SetSpeed(Motor_t* motor, float speed);

/**
 * @brief 停止电机
 * @param motor: 电机实体指针
 * @retval 0: 成功, -1: 参数错误
 */
int8_t Motor_Stop(Motor_t* motor);


/**
 * @brief 获取电机状态
 * @param motor: 电机实体指针
 * @retval 电机状态
 */
MotorState_t Motor_GetState(Motor_t* motor);

/**
 * @brief 使能/失能电机
 * @param motor: 电机实体指针
 * @param enable: 1-使能, 0-失能
 * @retval 0: 成功, -1: 参数错误
 */
int8_t Motor_Enable(Motor_t* motor, uint8_t enable);

#ifdef __cplusplus
}
#endif


#endif