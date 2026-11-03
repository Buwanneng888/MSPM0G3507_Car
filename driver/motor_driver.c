#include "motor_driver.h"
//DL_TimerG_setCaptureCompareValue(MOTOR_PWM_INST,400,GPIO_MOTOR_PWM_C1_IDX);

/* Private function prototypes -----------------------------------------------*/
static uint32_t Speed_To_PWM(float speed);
static int8_t Motor_ValidateParams(Motor_t *motor);
static int16_t Float_To_Speed1000(float speed);
/* Exported functions --------------------------------------------------------*/


/**
 * @brief 创建电机实体
 */
int8_t Motor_Create(Motor_t *motor,
                    GPTIMER_Regs *htim,
                    DL_TIMER_CC_INDEX channel,
                    GPIO_Regs *dir_port,
                    uint32_t dir_pin,
										GPIO_Regs *dir_port1,
                    uint32_t dir_pin1,
                    uint8_t reverse)
{

    // 初始化硬件配置
    motor->hw.htim = htim;
    motor->hw.channel = channel;
    motor->hw.dir_port = dir_port;
    motor->hw.dir_pin = dir_pin;
		motor->hw.dir_port1 = dir_port1;
    motor->hw.dir_pin1 = dir_pin1;
    // 初始化电机状态
    motor->speed = 0.0f;
    motor->state = MOTOR_STATE_STOP;
    motor->enable = 1;
    motor->reverse = reverse;
  
    //设置初始状态：停止（DIR=0, PWM=0）
		DL_GPIO_setPins(motor->hw.dir_port, motor->hw.dir_pin);
		DL_GPIO_clearPins(motor->hw.dir_port1, motor->hw.dir_pin1);
    DL_TimerG_setCaptureCompareValue(motor->hw.htim,0,motor->hw.channel);

    return 0;
}

/**
 * @brief 设置电机速度
 */
int8_t Motor_SetSpeed(Motor_t *motor, float speed)
{
    // 参数检查
    if (Motor_ValidateParams(motor) != 0)
    {
        return -1;
    }
    // 速度范围检查
    if (speed < MOTOR_SPEED_MIN || speed > MOTOR_SPEED_MAX)
    {
        return -1;
    }

    // 检查电机是否使能
    if (!motor->enable)
    {
        return -1;
    }
    // 保存速度值
    motor->speed = speed;
		
		int16_t speed_1000 = Float_To_Speed1000(speed);
    // 处理停止
    if (speed == 0.0f)
    {
				//TB6612停止
				DL_GPIO_clearPins(motor->hw.dir_port, motor->hw.dir_pin);
				DL_GPIO_clearPins(motor->hw.dir_port1, motor->hw.dir_pin1);
				DL_TimerG_setCaptureCompareValue(motor->hw.htim,0,motor->hw.channel);
        motor->state = MOTOR_STATE_STOP;
        return 0;
    }

    // A4950标准控制逻辑：
    // 正数：DIR=0, PWM=速度值
    // 负数：DIR=1, PWM=100+负数值
    // reverse参数：反装电机需要将速度取反
    
		int16_t actual_speed = motor->reverse ? -speed_1000 : speed_1000;  // 反装电机速度取反
		
		
		
//		int16_t actual_speed;
//			actual_speed = speed_1000;
//		
//		if (motor->reverse==1)
//			actual_speed = -speed_1000;
//		else 
//			actual_speed = speed_1000;
		
		
		uint32_t pwm_value;
		uint8_t dir_level;
    
		if (actual_speed < 0) 
		{
				// 负数（反转）
				dir_level = 1;
				pwm_value = Speed_To_PWM((float)-actual_speed);  // actual_speed是负数
				motor->state = MOTOR_STATE_BACKWARD;
		} 
		else 
		{
					// 正数（正转）
					dir_level = 0;
					pwm_value = Speed_To_PWM((float)actual_speed);
					motor->state = MOTOR_STATE_FORWARD;
		}
    
    // 设置DIR引脚
		if(dir_level == 0) //1 0 正传
		{
			 DL_GPIO_setPins(motor->hw.dir_port, motor->hw.dir_pin);
			 DL_GPIO_clearPins(motor->hw.dir_port1, motor->hw.dir_pin1);
		}
		else	// 0 1反转
		{
			 DL_GPIO_clearPins(motor->hw.dir_port, motor->hw.dir_pin);
			 DL_GPIO_setPins(motor->hw.dir_port1, motor->hw.dir_pin1);
		}
    // 设置PWM占空比
    DL_TimerG_setCaptureCompareValue(motor->hw.htim,pwm_value,motor->hw.channel);
    return 0;
}

/**
 * @brief 停止电机
 */
int8_t Motor_Stop(Motor_t *motor)
{
    // 参数检查
    if (Motor_ValidateParams(motor) != 0)
    {
        return -1;
    }

    
    DL_GPIO_clearPins(motor->hw.dir_port, motor->hw.dir_pin);
		DL_GPIO_clearPins(motor->hw.dir_port1, motor->hw.dir_pin1);
    DL_TimerG_setCaptureCompareValue(motor->hw.htim,0,motor->hw.channel);

    // 更新状态
    motor->speed = 0.0f;
    motor->state = MOTOR_STATE_STOP;

    return 0;
}

/**
 * @brief 获取电机状态
 */
MotorState_t Motor_GetState(Motor_t *motor)
{
    // 参数检查
    if (Motor_ValidateParams(motor) != 0)
    {
        return MOTOR_STATE_ERROR;
    }

    return motor->state;
}

/**
 * @brief 使能/失能电机
 */
int8_t Motor_Enable(Motor_t *motor, uint8_t enable)
{
    // 参数检查
    if (Motor_ValidateParams(motor) != 0)
    {
        return -1;
    }

    motor->enable = enable;

    // 如果失能，立即停止电机
    if (!enable)
    {
        
        DL_GPIO_clearPins(motor->hw.dir_port, motor->hw.dir_pin);
				DL_GPIO_clearPins(motor->hw.dir_port1, motor->hw.dir_pin1);
				DL_TimerG_setCaptureCompareValue(motor->hw.htim,0,motor->hw.channel);
        motor->speed = 0.0f;
        motor->state = MOTOR_STATE_STOP;
    }

    return 0;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 将速度值转换为PWM比较值
 * @param speed: 速度值 (-100 到 +100)
 * @retval PWM比较值 (0 到 MOTOR_PWM_PERIOD)
 */
static uint32_t Speed_To_PWM(float speed)
{
    float abs_speed;

    // 获取速度绝对值
    if (speed < 0)
    {
        abs_speed = (float)(-speed);
    }
    else
    {
        abs_speed = (float)speed;
    }

    // 如果速度为0，直接返回0
    if (abs_speed == 0)
    {
        return 0;
    }

    // 转换为PWM值：速度百分比 * PWM周期
    uint32_t pwm_value = abs_speed * MOTOR_PWM_PERIOD / MOTOR_MAX_PRECISION;

    // PWM阈值提升逻辑：如果计算出的PWM小于阈值，提升到阈值
    if (pwm_value > 0 && pwm_value < MOTOR_MIN_PWM_THRESHOLD)
    {
        pwm_value = MOTOR_MIN_PWM_THRESHOLD;
    }

    // 确保不超过最大值
    if (pwm_value > MOTOR_PWM_PERIOD)
    {
        pwm_value = MOTOR_PWM_PERIOD;
    }

    return pwm_value;
}

/**
 * @brief 验证电机参数有效性
 * @param motor: 电机实体指针
 * @retval 0: 有效, -1: 无效
 */
static int8_t Motor_ValidateParams(Motor_t *motor)
{
    if (motor == NULL ||
        motor->hw.htim == NULL ||
        motor->hw.dir_port == NULL)
    {
        return -1;
    }

    return 0;
}

/**
 * @brief 将浮点速度转换为1000级精度整数
 * @param speed: 浮点速度值 (-100.0 到 +100.0)
 * @retval 1000级精度整数 (-1000 到 +1000)
 */
static int16_t Float_To_Speed1000(float speed)
{
    return (int16_t)roundf(speed * MOTOR_PRECISION_SCALE);
}

