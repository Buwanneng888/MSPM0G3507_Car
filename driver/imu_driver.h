#ifndef __IMU_DRIVER_H
#define __IMU_DRIVER_H

#include <stdint.h>

typedef struct
{
    float acc_x;
    float acc_y;
    float acc_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float roll;
    float pitch;
    float yaw;
    uint32_t last_update_ms;
    uint32_t rx_count;
    uint32_t frame_count;
    uint32_t bad_header_count;
    uint32_t bad_type_count;
    uint32_t checksum_error;
    uint8_t acc_valid;
    uint8_t gyro_valid;
    uint8_t euler_valid;
} ImuData_t;

void imu_driver_init(void);
void imu_uart_rx_byte(uint8_t byte);
void imu_send_cmd(const char *cmd);
const ImuData_t *imu_get_data(void);
uint8_t imu_is_online(uint32_t now_ms);

#endif
