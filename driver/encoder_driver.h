#ifndef __ENCODER_DRIVER_H
#define __ENCODER_DRIVER_H

#include "bsp_system.h"

#define ENCODER_PRR (13*28*2)   //编码器转一圈的脉冲数  PRR * 减速比 * 倍频
#define PI 3.14159265f         
#define WHEEL_D  6.5f               //车轮直径
#define WHEEL_C (PI * WHEEL_D)      //车轮周长 
#define SAMPLE_TIME_S 0.01f     //采样时间,与调度器中任务执行时间一致(10ms)

//编码器结构体
typedef struct{
    uint8_t id;//编码器ID
    uint8_t reverse;//编码器方向是否反转 0-正常 1-反转
    int16_t count;//当前采样周期内编码器原始计数值
    int32_t total_count;//累计总计数值
    float speed_cm_s;//编码器速度
}encoder;

/**
 * @brief 编码器任务函数
 * 
 */
void encoder_task(void);
/**
 * @brief 编码器数据更新 周期性调用
 * 
 * @param encoder 
 */
void encoder_update(encoder *encoder);
/**
 * @brief 编码器配置
 * 
 */
void encoder_config(void);

extern encoder encoder_left;
extern encoder encoder_right;
#endif