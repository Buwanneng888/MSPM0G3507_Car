/**
 * @file icm42688_driver.c
 * @brief ICM42688 driver for MSPM0G3507 using software I2C on PA0/PA1.
 *
 * Wiring from this project:
 *   PA0 -> ICM42688 SDA
 *   PA1 -> ICM42688 SCL
 */

#include "icm42688_driver.h"
#include "ti_msp_dl_config.h"
#include "Time.h"

#define ICM_SDA_PORT       (IMU_PORT)
#define ICM_SDA_PIN        (IMU_IMU_SDA_PIN)
#define ICM_SDA_IOMUX      (IMU_IMU_SDA_IOMUX)
#define ICM_SCL_PORT       (IMU_PORT)
#define ICM_SCL_PIN        (IMU_IMU_SCL_PIN)
#define ICM_SCL_IOMUX      (IMU_IMU_SCL_IOMUX)

#define I2C_ACK_TIMEOUT    (80U)
#define I2C_CLOCK_TIMEOUT  (80U)
#define I2C_DELAY_US       (3U)

static float g_acc_sensitivity = 4000.0f / 32768.0f;
static float g_gyro_sensitivity = 1000.0f / 32768.0f;
static uint8_t g_i2c_address = ICM42688_I2C_ADDR_AD0_HIGH;
static uint8_t g_who_am_i = 0U;

static void i2c_delay(void)
{
    delay_us(I2C_DELAY_US);
}

/* Open-drain behavior is implemented by driving low or releasing the pin. */
static void sda_low(void)
{
    DL_GPIO_initDigitalOutput(ICM_SDA_IOMUX);
    DL_GPIO_clearPins(ICM_SDA_PORT, ICM_SDA_PIN);
    DL_GPIO_enableOutput(ICM_SDA_PORT, ICM_SDA_PIN);
}

static void sda_release(void)
{
    DL_GPIO_disableOutput(ICM_SDA_PORT, ICM_SDA_PIN);
    DL_GPIO_initDigitalInput(ICM_SDA_IOMUX);
}

static void scl_low(void)
{
    DL_GPIO_initDigitalOutput(ICM_SCL_IOMUX);
    DL_GPIO_clearPins(ICM_SCL_PORT, ICM_SCL_PIN);
    DL_GPIO_enableOutput(ICM_SCL_PORT, ICM_SCL_PIN);
}

static void scl_release(void)
{
    DL_GPIO_disableOutput(ICM_SCL_PORT, ICM_SCL_PIN);
    DL_GPIO_initDigitalInput(ICM_SCL_IOMUX);
}

static uint8_t sda_read(void)
{
    return ((DL_GPIO_readPins(ICM_SDA_PORT, ICM_SDA_PIN) & ICM_SDA_PIN) != 0U) ? 1U : 0U;
}

static uint8_t scl_read(void)
{
    return ((DL_GPIO_readPins(ICM_SCL_PORT, ICM_SCL_PIN) & ICM_SCL_PIN) != 0U) ? 1U : 0U;
}

static uint8_t wait_scl_high(void)
{
    uint16_t timeout = I2C_CLOCK_TIMEOUT;

    scl_release();
    while ((scl_read() == 0U) && (timeout > 0U)) {
        timeout--;
        delay_us(1U);
    }

    return (timeout == 0U) ? 1U : 0U;
}

static void i2c_stop(void)
{
    sda_low();
    i2c_delay();
    (void)wait_scl_high();
    i2c_delay();
    sda_release();
    i2c_delay();
}

static uint8_t i2c_start(void)
{
    sda_release();
    if (wait_scl_high() != 0U) {
        return 1U;
    }
    i2c_delay();

    if (sda_read() == 0U) {
        return 1U;
    }

    sda_low();
    i2c_delay();
    scl_low();
    i2c_delay();
    return 0U;
}

static uint8_t i2c_write_byte(uint8_t data)
{
    uint8_t i;
    uint16_t timeout;
    uint8_t nack;

    for (i = 0U; i < 8U; i++) {
        if ((data & 0x80U) != 0U) {
            sda_release();
        } else {
            sda_low();
        }
        i2c_delay();

        if (wait_scl_high() != 0U) {
            scl_low();
            sda_release();
            return 1U;
        }
        i2c_delay();
        scl_low();
        i2c_delay();
        data <<= 1;
    }

    sda_release();
    i2c_delay();
    if (wait_scl_high() != 0U) {
        scl_low();
        return 1U;
    }

    timeout = I2C_ACK_TIMEOUT;
    while ((sda_read() != 0U) && (timeout > 0U)) {
        timeout--;
        delay_us(1U);
    }
    nack = (timeout == 0U) ? 1U : 0U;

    i2c_delay();
    scl_low();
    i2c_delay();
    return nack;
}

static uint8_t i2c_read_byte(uint8_t send_nack, uint8_t *data)
{
    uint8_t i;
    uint8_t value = 0U;

    if (data == 0) {
        return 1U;
    }

    sda_release();
    for (i = 0U; i < 8U; i++) {
        value <<= 1;
        if (wait_scl_high() != 0U) {
            scl_low();
            return 1U;
        }
        i2c_delay();
        if (sda_read() != 0U) {
            value |= 1U;
        }
        scl_low();
        i2c_delay();
    }

    if (send_nack != 0U) {
        sda_release();
    } else {
        sda_low();
    }
    i2c_delay();
    if (wait_scl_high() != 0U) {
        scl_low();
        sda_release();
        return 1U;
    }
    i2c_delay();
    scl_low();
    sda_release();
    i2c_delay();

    *data = value;
    return 0U;
}

static void i2c_bus_recover(void)
{
    uint8_t i;

    sda_release();
    scl_release();
    delay_us(10U);

    if (sda_read() == 0U) {
        for (i = 0U; i < 9U; i++) {
            scl_low();
            i2c_delay();
            (void)wait_scl_high();
            i2c_delay();
            if (sda_read() != 0U) {
                break;
            }
        }
    }

    i2c_stop();
}

static int8_t i2c_write_register(uint8_t address, uint8_t reg, uint8_t value)
{
    if (i2c_start() != 0U) {
        i2c_bus_recover();
        return ICM42688_ERROR_I2C;
    }
    if (i2c_write_byte((uint8_t)(address << 1)) != 0U) {
        i2c_stop();
        return ICM42688_ERROR_I2C;
    }
    if (i2c_write_byte(reg) != 0U) {
        i2c_stop();
        return ICM42688_ERROR_I2C;
    }
    if (i2c_write_byte(value) != 0U) {
        i2c_stop();
        return ICM42688_ERROR_I2C;
    }
    i2c_stop();
    return ICM42688_OK;
}

static int8_t i2c_read_registers(uint8_t address, uint8_t reg,
                                 uint8_t *buffer, uint16_t length)
{
    uint16_t i;

    if ((buffer == 0) || (length == 0U)) {
        return ICM42688_ERROR_PARAM;
    }

    if (i2c_start() != 0U) {
        i2c_bus_recover();
        return ICM42688_ERROR_I2C;
    }
    if (i2c_write_byte((uint8_t)(address << 1)) != 0U) {
        i2c_stop();
        return ICM42688_ERROR_I2C;
    }
    if (i2c_write_byte(reg) != 0U) {
        i2c_stop();
        return ICM42688_ERROR_I2C;
    }

    if (i2c_start() != 0U) {
        i2c_stop();
        return ICM42688_ERROR_I2C;
    }
    if (i2c_write_byte((uint8_t)((address << 1) | 1U)) != 0U) {
        i2c_stop();
        return ICM42688_ERROR_I2C;
    }

    for (i = 0U; i < length; i++) {
        uint8_t nack = (i == (uint16_t)(length - 1U)) ? 1U : 0U;
        if (i2c_read_byte(nack, &buffer[i]) != 0U) {
            i2c_stop();
            return ICM42688_ERROR_I2C;
        }
    }

    i2c_stop();
    return ICM42688_OK;
}

static int8_t icm_read_reg(uint8_t reg, uint8_t *value)
{
    return i2c_read_registers(g_i2c_address, reg, value, 1U);
}

static int8_t icm_read_regs(uint8_t reg, uint8_t *buffer, uint16_t length)
{
    return i2c_read_registers(g_i2c_address, reg, buffer, length);
}

static int8_t icm_write_reg(uint8_t reg, uint8_t value)
{
    return i2c_write_register(g_i2c_address, reg, value);
}

static int8_t probe_sensor(void)
{
    static const uint8_t addresses[2] = {
        ICM42688_I2C_ADDR_AD0_HIGH,
        ICM42688_I2C_ADDR_AD0_LOW
    };
    uint8_t i;
    uint8_t id;

    for (i = 0U; i < 2U; i++) {
        id = 0U;
        if (i2c_read_registers(addresses[i], ICM42688_REG_WHO_AM_I, &id, 1U) == ICM42688_OK) {
            if (id == ICM42688_WHO_AM_I_VALUE) {
                g_i2c_address = addresses[i];
                g_who_am_i = id;
                return ICM42688_OK;
            }
        }
    }

    g_who_am_i = 0U;
    return ICM42688_ERROR_ID;
}

float icm42688_get_accel_sensitivity(uint8_t scale)
{
    switch (scale) {
    case AFS_2G:  g_acc_sensitivity = 2000.0f / 32768.0f; break;
    case AFS_4G:  g_acc_sensitivity = 4000.0f / 32768.0f; break;
    case AFS_8G:  g_acc_sensitivity = 8000.0f / 32768.0f; break;
    case AFS_16G: g_acc_sensitivity = 16000.0f / 32768.0f; break;
    default:      g_acc_sensitivity = 4000.0f / 32768.0f; break;
    }
    return g_acc_sensitivity;
}

float icm42688_get_gyro_sensitivity(uint8_t scale)
{
    switch (scale) {
    case GFS_15_125DPS: g_gyro_sensitivity = 15.125f / 32768.0f; break;
    case GFS_31_25DPS:  g_gyro_sensitivity = 31.25f / 32768.0f; break;
    case GFS_62_5DPS:   g_gyro_sensitivity = 62.5f / 32768.0f; break;
    case GFS_125DPS:    g_gyro_sensitivity = 125.0f / 32768.0f; break;
    case GFS_250DPS:    g_gyro_sensitivity = 250.0f / 32768.0f; break;
    case GFS_500DPS:    g_gyro_sensitivity = 500.0f / 32768.0f; break;
    case GFS_1000DPS:   g_gyro_sensitivity = 1000.0f / 32768.0f; break;
    case GFS_2000DPS:   g_gyro_sensitivity = 2000.0f / 32768.0f; break;
    default:            g_gyro_sensitivity = 1000.0f / 32768.0f; break;
    }
    return g_gyro_sensitivity;
}

int8_t icm42688_init(void)
{
    uint8_t value;
    int8_t status;

    g_who_am_i = 0U;
    i2c_bus_recover();
    delay_ms(10U);

    status = probe_sensor();
    if (status != ICM42688_OK) {
        return status;
    }

    if (icm_write_reg(ICM42688_REG_BANK_SEL, 0x00U) != ICM42688_OK) {
        return ICM42688_ERROR_I2C;
    }
    if (icm_write_reg(ICM42688_REG_DEVICE_CONFIG, 0x01U) != ICM42688_OK) {
        return ICM42688_ERROR_I2C;
    }
    delay_ms(3U);

    status = probe_sensor();
    if (status != ICM42688_OK) {
        return status;
    }

    if (icm_write_reg(ICM42688_REG_BANK_SEL, 0x00U) != ICM42688_OK) {
        return ICM42688_ERROR_I2C;
    }

    (void)icm42688_get_accel_sensitivity(ICM42688_ACCEL_FS);
    (void)icm42688_get_gyro_sensitivity(ICM42688_GYRO_FS);

    value = (uint8_t)((ICM42688_ACCEL_FS << 5) | ICM42688_ACCEL_ODR);
    if (icm_write_reg(ICM42688_REG_ACCEL_CONFIG0, value) != ICM42688_OK) {
        return ICM42688_ERROR_I2C;
    }

    value = (uint8_t)((ICM42688_GYRO_FS << 5) | ICM42688_GYRO_ODR);
    if (icm_write_reg(ICM42688_REG_GYRO_CONFIG0, value) != ICM42688_OK) {
        return ICM42688_ERROR_I2C;
    }

    /* Temperature enabled, gyro low-noise, accelerometer low-noise. */
    if (icm_write_reg(ICM42688_REG_PWR_MGMT0, 0x0FU) != ICM42688_OK) {
        return ICM42688_ERROR_I2C;
    }
    delay_ms(50U);

    if (icm_read_reg(ICM42688_REG_WHO_AM_I, &value) != ICM42688_OK) {
        return ICM42688_ERROR_I2C;
    }
    if (value != ICM42688_WHO_AM_I_VALUE) {
        return ICM42688_ERROR_ID;
    }

    if (icm_read_reg(ICM42688_REG_ACCEL_CONFIG0, &value) != ICM42688_OK) {
        return ICM42688_ERROR_I2C;
    }
    if (value != (uint8_t)((ICM42688_ACCEL_FS << 5) | ICM42688_ACCEL_ODR)) {
        return ICM42688_ERROR_CONFIG;
    }

    if (icm_read_reg(ICM42688_REG_GYRO_CONFIG0, &value) != ICM42688_OK) {
        return ICM42688_ERROR_I2C;
    }
    if (value != (uint8_t)((ICM42688_GYRO_FS << 5) | ICM42688_GYRO_ODR)) {
        return ICM42688_ERROR_CONFIG;
    }

    return ICM42688_OK;
}

static int16_t bytes_to_i16(uint8_t high, uint8_t low)
{
    return (int16_t)(((uint16_t)high << 8) | (uint16_t)low);
}

int8_t icm42688_get_raw_data(icm42688_real_data_t *acc,
                             icm42688_real_data_t *gyro)
{
    uint8_t buffer[12];
    icm42688_raw_data_t acc_raw;
    icm42688_raw_data_t gyro_raw;
    int8_t status;

    if ((acc == 0) || (gyro == 0)) {
        return ICM42688_ERROR_PARAM;
    }

    status = icm_read_regs(ICM42688_REG_ACCEL_DATA_X1, buffer, 12U);
    if (status != ICM42688_OK) {
        return status;
    }

    acc_raw.x = bytes_to_i16(buffer[0], buffer[1]);
    acc_raw.y = bytes_to_i16(buffer[2], buffer[3]);
    acc_raw.z = bytes_to_i16(buffer[4], buffer[5]);
    gyro_raw.x = bytes_to_i16(buffer[6], buffer[7]);
    gyro_raw.y = bytes_to_i16(buffer[8], buffer[9]);
    gyro_raw.z = bytes_to_i16(buffer[10], buffer[11]);

    /* 0x8000 is the sensor's invalid-data marker during startup. */
    if ((acc_raw.x == (int16_t)0x8000) || (acc_raw.y == (int16_t)0x8000) ||
        (acc_raw.z == (int16_t)0x8000) || (gyro_raw.x == (int16_t)0x8000) ||
        (gyro_raw.y == (int16_t)0x8000) || (gyro_raw.z == (int16_t)0x8000)) {
        return ICM42688_ERROR_DATA;
    }

    acc->x = (float)acc_raw.x * g_acc_sensitivity;
    acc->y = (float)acc_raw.y * g_acc_sensitivity;
    acc->z = (float)acc_raw.z * g_acc_sensitivity;

    gyro->x = (float)gyro_raw.x * g_gyro_sensitivity;
    gyro->y = (float)gyro_raw.y * g_gyro_sensitivity;
    gyro->z = (float)gyro_raw.z * g_gyro_sensitivity;

    return ICM42688_OK;
}

int8_t icm42688_get_accel(icm42688_raw_data_t *acc)
{
    icm42688_real_data_t accel;
    icm42688_real_data_t gyro;
    int8_t status;

    if (acc == 0) {
        return ICM42688_ERROR_PARAM;
    }
    status = icm42688_get_raw_data(&accel, &gyro);
    if (status != ICM42688_OK) {
        return status;
    }

    acc->x = (int16_t)accel.x;
    acc->y = (int16_t)accel.y;
    acc->z = (int16_t)accel.z;
    return ICM42688_OK;
}

int8_t icm42688_get_gyro(icm42688_raw_data_t *gyro)
{
    icm42688_real_data_t accel;
    icm42688_real_data_t gyro_real;
    int8_t status;

    if (gyro == 0) {
        return ICM42688_ERROR_PARAM;
    }
    status = icm42688_get_raw_data(&accel, &gyro_real);
    if (status != ICM42688_OK) {
        return status;
    }

    gyro->x = (int16_t)gyro_real.x;
    gyro->y = (int16_t)gyro_real.y;
    gyro->z = (int16_t)gyro_real.z;
    return ICM42688_OK;
}

int8_t icm42688_get_temperature(int16_t *temperature_c)
{
    uint8_t buffer[2];
    int16_t raw;
    int8_t status;

    if (temperature_c == 0) {
        return ICM42688_ERROR_PARAM;
    }

    status = icm_read_regs(ICM42688_REG_TEMP_DATA1, buffer, 2U);
    if (status != ICM42688_OK) {
        return status;
    }

    raw = bytes_to_i16(buffer[0], buffer[1]);
    if (raw == (int16_t)0x8000) {
        return ICM42688_ERROR_DATA;
    }
    *temperature_c = (int16_t)(((float)raw / 132.48f) + 25.0f);
    return ICM42688_OK;
}

uint8_t icm42688_get_who_am_i(void)
{
    return g_who_am_i;
}

uint8_t icm42688_get_i2c_address(void)
{
    return g_i2c_address;
}
