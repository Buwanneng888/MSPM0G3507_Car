#ifndef __USER_PID_H
#define __USER_PID_H

#include "bsp_system.h"

/** PID 参数配置；输入/输出单位由对应控制环决定。 */
typedef struct
{
    float kp;       /**< 比例系数：决定当前误差的响应强度。 */
    float ki;       /**< 积分系数：用于消除长期静差。 */
    float kd;       /**< 微分系数：抑制误差快速变化和超调。 */
    float out_min;  /**< 控制器允许的最小输出。 */
    float out_max;  /**< 控制器允许的最大输出。 */
} PidParams_t;

extern int basic_speed;                         /**< 小车基础目标速度，单位：cm/s。 */
extern volatile bool pid_running;              /**< PID 总运行开关：false 时立即停止电机。 */
extern volatile bool heading_contrl_enable;    /**< 控制模式：false=循迹，true=航向闭环。 */

/** 10ms 控制任务：执行循迹/航向外环、左右轮速度环和电机输出。 */
void PID_Task(void);

/** 航向调试打印任务。 */
void heading_debug_task(void);

/** 使用按键按 90° 步进修改航向目标的调试任务。 */
void heading_rest_task(void);

/** 初始化左右轮速度环、循迹环和航向环。 */
void PID_Init(void);

/** 执行一次航向外环计算并更新左右轮目标速度。 */
void Heading_PID_control(void);

/** 开启或关闭 PID；关闭时立即将左右电机输出清零。 */
void PID_SetRunning(bool enable);

/** 设置基础速度，输入范围会限制到 0~120cm/s。 */
void PID_SetBasicSpeed(int speed);

/** 切换循迹/航向控制模式，并清除各 PID 的历史状态。 */
void PID_SetHeadingControl(bool enable);

/** 设置航向闭环目标，单位：度，内部归一化到 [-180°, 180°)。 */
void PID_SetHeadingTarget(float target_deg);

/** 返回航向目标与当前航向之间的最短有符号误差，单位：度。 */
float PID_GetHeadingError(void);

#endif
