#ifndef __USER_MOTOR_H
#define __USER_MOTOR_H


#include "bsp_system.h"
void user_motor_init(void);

void motor_set_l(float speed);
void motor_set_r(float speed);
void motor_stop(void);


#endif