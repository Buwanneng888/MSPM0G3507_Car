#ifndef __IMU_IMU_APP_H__
#define __IMU_IMU_APP_H__

#include <stdint.h>
#include "bsp_system.h"

typedef struct {
    float x;
    float y;
    float z;
} xyz_f_t;

typedef struct {
    xyz_f_t acc;       /* mg */
    xyz_f_t gyro;      /* degree/s */
} imu_imu_sensor_t;

typedef enum {
    IMU_IMU_STATE_OFFLINE = 0,
    IMU_IMU_STATE_CALIBRATING,
    IMU_IMU_STATE_READY
} imu_imu_state_t;

typedef struct {
    imu_imu_sensor_t raw;

    /* x=yaw, y=pitch, z=roll, in degree. */
    xyz_f_t euler;
    float quat[4];
    float gyro_z_nobias;

    xyz_f_t gyro_offset;
    float kp;

    uint32_t last_update_ms;
    uint32_t last_sample_ms;
    uint32_t last_good_ms;
    uint32_t init_time_ms;
    uint32_t sample_count;
    uint32_t read_error_count;
    uint32_t reinit_count;

    uint8_t who_am_i;
    uint8_t i2c_address;
    uint8_t sensor_ok;
    uint8_t calibrated;
    uint8_t ready;
    imu_imu_state_t state;
} imu_imu_data_t;

extern imu_imu_data_t imu_imu;

void imu_imu_init(void);
void imu_imu_task(void);

const imu_imu_data_t *imu_imu_get_data(void);
float imu_imu_get_yaw(void);
float imu_imu_get_gyro_z(void);
uint8_t imu_imu_app_is_ready(void);

#endif /* __IMU_IMU_APP_H__ */
