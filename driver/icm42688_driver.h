#ifndef __ICM42688_DRIVER_H__
#define __ICM42688_DRIVER_H__

#include <stdint.h>

/* ICM42688 I2C address and identity. */
#define ICM42688_I2C_ADDR_AD0_LOW     (0x68U)
#define ICM42688_I2C_ADDR_AD0_HIGH    (0x69U)
#define ICM42688_WHO_AM_I_VALUE       (0x47U)

/* Backward-compatible names used by the previous migration. */
#define ICM42688_ADDRESS              ICM42688_I2C_ADDR_AD0_HIGH
#define ICM42688_ID                   ICM42688_WHO_AM_I_VALUE

/* Bank 0 registers used by this driver. */
#define ICM42688_REG_DEVICE_CONFIG    (0x11U)
#define ICM42688_REG_TEMP_DATA1       (0x1DU)
#define ICM42688_REG_ACCEL_DATA_X1    (0x1FU)
#define ICM42688_REG_GYRO_DATA_X1     (0x25U)
#define ICM42688_REG_PWR_MGMT0        (0x4EU)
#define ICM42688_REG_GYRO_CONFIG0     (0x4FU)
#define ICM42688_REG_ACCEL_CONFIG0    (0x50U)
#define ICM42688_REG_WHO_AM_I         (0x75U)
#define ICM42688_REG_BANK_SEL         (0x76U)

#define ICM42688_DEVICE_CONFIG        ICM42688_REG_DEVICE_CONFIG
#define ICM42688_TEMP_DATA1           ICM42688_REG_TEMP_DATA1
#define ICM42688_ACCEL_DATA_X1        ICM42688_REG_ACCEL_DATA_X1
#define ICM42688_GYRO_DATA_X1         ICM42688_REG_GYRO_DATA_X1
#define ICM42688_PWR_MGMT0            ICM42688_REG_PWR_MGMT0
#define ICM42688_GYRO_CONFIG0         ICM42688_REG_GYRO_CONFIG0
#define ICM42688_ACCEL_CONFIG0        ICM42688_REG_ACCEL_CONFIG0
#define ICM42688_WHO_AM_I             ICM42688_REG_WHO_AM_I

/* Full-scale register values. */
#define AFS_2G                        (0x03U)
#define AFS_4G                        (0x02U)
#define AFS_8G                        (0x01U)
#define AFS_16G                       (0x00U)

#define GFS_2000DPS                   (0x00U)
#define GFS_1000DPS                   (0x01U)
#define GFS_500DPS                    (0x02U)
#define GFS_250DPS                    (0x03U)
#define GFS_125DPS                    (0x04U)
#define GFS_62_5DPS                   (0x05U)
#define GFS_31_25DPS                  (0x06U)
#define GFS_15_125DPS                 (0x07U)

/* Output data-rate register values. */
#define AODR_1000HZ                   (0x06U)
#define AODR_200HZ                    (0x07U)
#define AODR_100HZ                    (0x08U)
#define AODR_50HZ                     (0x09U)
#define AODR_25HZ                     (0x0AU)
#define AODR_12_5HZ                   (0x0BU)
#define AODR_500HZ                    (0x0FU)

#define GODR_1000HZ                   (0x06U)
#define GODR_200HZ                    (0x07U)
#define GODR_100HZ                    (0x08U)
#define GODR_50HZ                     (0x09U)
#define GODR_25HZ                     (0x0AU)
#define GODR_12_5HZ                   (0x0BU)
#define GODR_500HZ                    (0x0FU)

#define AODR_1000Hz AODR_1000HZ
#define AODR_200Hz  AODR_200HZ
#define AODR_100Hz  AODR_100HZ
#define AODR_50Hz   AODR_50HZ
#define AODR_25Hz   AODR_25HZ
#define AODR_12_5Hz AODR_12_5HZ
#define AODR_500Hz  AODR_500HZ
#define GODR_1000Hz GODR_1000HZ
#define GODR_200Hz  GODR_200HZ
#define GODR_100Hz  GODR_100HZ
#define GODR_50Hz   GODR_50HZ
#define GODR_25Hz   GODR_25HZ
#define GODR_12_5Hz GODR_12_5HZ
#define GODR_500Hz  GODR_500HZ

/* Project configuration: 4 g, 1000 dps, 100 Hz. */
#define ICM42688_ACCEL_FS             AFS_4G
#define ICM42688_GYRO_FS              GFS_1000DPS
#define ICM42688_ACCEL_ODR            AODR_100HZ
#define ICM42688_GYRO_ODR             GODR_100HZ

typedef enum {
    ICM42688_OK              = 0,
    ICM42688_ERROR_PARAM     = -1,
    ICM42688_ERROR_I2C       = -2,
    ICM42688_ERROR_ID        = -3,
    ICM42688_ERROR_CONFIG    = -4,
    ICM42688_ERROR_DATA      = -5
} icm42688_status_t;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} icm42688_raw_data_t;

typedef struct {
    float x;
    float y;
    float z;
} icm42688_real_data_t;

/* Initialize the sensor and verify WHO_AM_I/configuration. */
int8_t icm42688_init(void);

/* Converted outputs: acceleration in mg, gyro in degree/s. */
int8_t icm42688_get_raw_data(icm42688_real_data_t *acc,
                             icm42688_real_data_t *gyro);

/* Compatibility APIs. These return converted values rounded to int16_t. */
int8_t icm42688_get_accel(icm42688_raw_data_t *acc);
int8_t icm42688_get_gyro(icm42688_raw_data_t *gyro);
int8_t icm42688_get_temperature(int16_t *temperature_c);

float icm42688_get_accel_sensitivity(uint8_t scale);
float icm42688_get_gyro_sensitivity(uint8_t scale);

uint8_t icm42688_get_who_am_i(void);
uint8_t icm42688_get_i2c_address(void);

#endif /* __ICM42688_DRIVER_H__ */
