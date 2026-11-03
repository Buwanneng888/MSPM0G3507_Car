/* Licence
 * Author:  Buwannengfeng
 * Version: V0.10
 * Time:    2026/10/06
 */

#include "user_motor.h"

Motor_t left_motor;
Motor_t right_motor;

void user_motor_init(void)
{
    my_printf(UART_0_INST, "motor init ok\r\n");
    Motor_Create(&left_motor,
                 MOTOR_PWM_LEFT_INST,
                 GPIO_MOTOR_PWM_LEFT_C0_IDX,
                 MOTOR_DIR_LEFT1_PORT,
                 MOTOR_DIR_LEFT1_PIN_0_PIN,
                 MOTOR_DIR_LEFT2_PORT,
                 MOTOR_DIR_LEFT2_PIN_1_PIN,
                 0);

    Motor_Create(&right_motor,
                 MOTOR_PWM_RIGHT_INST,
                 GPIO_MOTOR_PWM_RIGHT_C0_IDX,
                 MOTOR_DIR_RIGHT1_PORT,
                 MOTOR_DIR_RIGHT1_PIN_2_PIN,
                 MOTOR_DIR_RIGHT2_PORT,
                 MOTOR_DIR_RIGHT2_PIN_3_PIN,
                 1);
}

void motor_set_l(float speed)
{
    Motor_SetSpeed(&left_motor, speed);
}

void motor_set_r(float speed)
{
    Motor_SetSpeed(&right_motor, speed);
}

//停止左右电机，用于要求2急停
void motor_stop(void)
{
	Motor_Stop(&left_motor);
	Motor_Stop(&right_motor);
}
