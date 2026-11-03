#ifndef __IMU_APP_H
#define __IMU_APP_H

#include <stdint.h>

void imu_app_init(void);
void imu_app_task(void);
void imu_zero_yaw(void);
float imu_get_yaw(void);
float imu_get_yaw_error(float target_yaw);
float imu_get_gyro_z(void);
uint8_t imu_app_is_ready(void);

#endif
