#include "encoder_driver.h"
#define ENCODER_NUM (2)     //编码器数量
//编码器原始计数值
volatile int16_t encoder_get_count[ENCODER_NUM];//left:0 right:1

encoder encoder_left;
encoder encoder_right;
/**
 * @brief 编码器计数中断处理函数
 * 
 */
static void encoder_func(void)
{
    //如果是左轮A相中断触发
    if(DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT,ENCODER_left_a_PIN))
    {
        //判断B相当前电平状态，执行对应加减逻辑
        if(DL_GPIO_readPins(ENCODER_PORT,ENCODER_left_b_PIN) == 0)
        {
            //编码器计数+1
            encoder_get_count[0]++;
        }
        else
        {
            //编码器计数-1
            encoder_get_count[0]--;
        }
        //清除A相中断标志位
        DL_GPIO_clearInterruptStatus(ENCODER_PORT,ENCODER_left_a_PIN);
    }
    //如果是左轮B相中断触发
      if(DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT,ENCODER_left_b_PIN))
    {
        //判断A相当前电平状态，执行对应加减逻辑
        if(DL_GPIO_readPins(ENCODER_PORT,ENCODER_left_a_PIN) == 0)
        {
            encoder_get_count[0]++;
        }
        else
        {
            encoder_get_count[0]--;
        }
        //清除B相中断标志位
        DL_GPIO_clearInterruptStatus(ENCODER_PORT,ENCODER_left_b_PIN);
    }
    //如果是右轮A相中断触发
    if(DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT,ENCODER_right_a_PIN))
    {
        //判断B相当前电平状态，执行对应加减逻辑
        if(DL_GPIO_readPins(ENCODER_PORT,ENCODER_right_b_PIN) == 0)
        {
            //编码器计数+1
            encoder_get_count[1]++;
        }
        else
        {
            //编码器计数-1
            encoder_get_count[1]--;
        }
        //清除A相中断标志位
        DL_GPIO_clearInterruptStatus(ENCODER_PORT,ENCODER_right_a_PIN);
    }
    //如果是右轮B相中断触发
    if(DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT,ENCODER_right_b_PIN))
    {
        //判断A相当前电平状态，执行对应加减逻辑
        if(DL_GPIO_readPins(ENCODER_PORT,ENCODER_right_a_PIN) == 0)
        {
            encoder_get_count[1]++;
        }
        else
        {
            encoder_get_count[1]--;
        }
        //清除B相中断标志位
        DL_GPIO_clearInterruptStatus(ENCODER_PORT,ENCODER_right_b_PIN);
    }
}
/**
 * @brief 外部中断服务函数
 * 
 */
void GROUP1_IRQHandler(void)
{
    //判断中断源并调用对应处理函数
    switch( DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1) )
    {
        //编码器中断源
        case ENCODER_INT_IIDX:
            encoder_func();
        break;
    }
}
/**
 * @brief 编码器初始化
 * 
 * @param encoder 编码器结构体指针
 * @param id 编码器编号（0左/1右）
 * @param reverse 计数方向反转使能(0正常 1反转)
 */
void encoder_init(encoder *encoder,uint8_t id,uint8_t reverse)
{
    encoder->id = id;
    encoder->reverse = reverse;
    
    encoder->count = 0;
    encoder->total_count = 0;
    encoder->speed_cm_s = 0;
}
/**
 * @brief 编码器数据刷新，周期调用
 * 
 * @param encoder 编码器结构体指针
 */
void encoder_update(encoder *encoder)
{
    //读取原始计数值快照
    int16_t count_snapshot;

    __disable_irq();
    count_snapshot = encoder_get_count[encoder->id];
    encoder_get_count[encoder->id] = 0;
    __enable_irq();

    encoder->count = count_snapshot;
    //处理编码器方向反转
    encoder->count = (encoder->reverse == 0 ? encoder->count : -encoder->count);
    //清零原始计数变量
    
		//my_printf(UART_0_INST,"count:%d\r\n",encoder->count);
    //累计总脉冲计数
    encoder->total_count += encoder->count;
    //速度计算(单位cm/s)
    //(脉冲值 / 单圈脉冲数PRR) = 车轮转动圈数
    //转动圈数 × 车轮周长 = 行驶距离
    //行驶距离 ÷ 采样周期 = 瞬时速度
    encoder->speed_cm_s = (float)encoder->count / ENCODER_PRR * WHEEL_C / SAMPLE_TIME_S;
//		encoder->speed_cm_s = encoder->count;
}
/**
 * @brief 编码器周期任务函数
 * 
 */
__WEAK void encoder_task(void)
{
    encoder_update(&encoder_left);
    encoder_update(&encoder_right);
}
/**
 * @brief 编码器整体配置初始化
 * 
 */
void encoder_config(void)
{
    my_printf(UART_0_INST, "encoder init ok\r\n");
    encoder_init(&encoder_left,0,1);
    encoder_init(&encoder_right,1,0);
}