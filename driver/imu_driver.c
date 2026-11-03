#include "bsp_system.h"
#include "imu_driver.h"

#define IMU_FRAME_LEN 11
#define IMU_HEADER 0x55
#define IMU_PACKET_ACC 0x51
#define IMU_PACKET_GYRO 0x52
#define IMU_PACKET_EULER 0x53
#define IMU_ONLINE_TIMEOUT_MS 100

static ImuData_t imu_data;
static uint8_t imu_frame[IMU_FRAME_LEN];
static uint8_t imu_frame_idx;

static int16_t imu_i16_from_le(uint8_t low, uint8_t high)
{
    return (int16_t)((uint16_t)low | ((uint16_t)high << 8));
}

static uint8_t imu_checksum_ok(const uint8_t *frame)
{
    uint8_t sum = 0;

    for (uint8_t i = 0; i < IMU_FRAME_LEN - 1; i++)
    {
        sum += frame[i];
    }

    return sum == frame[IMU_FRAME_LEN - 1];
}

static void imu_parse_frame(const uint8_t *frame)
{
    int16_t raw_x = imu_i16_from_le(frame[2], frame[3]);
    int16_t raw_y = imu_i16_from_le(frame[4], frame[5]);
    int16_t raw_z = imu_i16_from_le(frame[6], frame[7]);

    if (!imu_checksum_ok(frame))
    {
        imu_data.checksum_error++;
        return;
    }

    switch (frame[1])
    {
        case IMU_PACKET_ACC:
            imu_data.acc_x = (float)raw_x * 16.0f / 32768.0f;
            imu_data.acc_y = (float)raw_y * 16.0f / 32768.0f;
            imu_data.acc_z = (float)raw_z * 16.0f / 32768.0f;
            imu_data.acc_valid = 1;
            break;

        case IMU_PACKET_GYRO:
            imu_data.gyro_x = (float)raw_x * 2000.0f / 32768.0f;
            imu_data.gyro_y = (float)raw_y * 2000.0f / 32768.0f;
            imu_data.gyro_z = (float)raw_z * 2000.0f / 32768.0f;
            imu_data.gyro_valid = 1;
            break;

        case IMU_PACKET_EULER:
            imu_data.roll = (float)raw_x * 180.0f / 32768.0f;
            imu_data.pitch = (float)raw_y * 180.0f / 32768.0f;
            imu_data.yaw = (float)raw_z * 180.0f / 32768.0f;
            imu_data.euler_valid = 1;
            break;

        default:
            return;
    }

    imu_data.frame_count++;
    imu_data.last_update_ms = uwTick;
}

void imu_driver_init(void)
{
    memset(&imu_data, 0, sizeof(imu_data));
    memset(imu_frame, 0, sizeof(imu_frame));
    imu_frame_idx = 0;
}

void imu_uart_rx_byte(uint8_t byte)
{
    imu_data.rx_count++;

    if (imu_frame_idx == 0)
    {
        if (byte != IMU_HEADER)
        {
            imu_data.bad_header_count++;
            return;
        }

        imu_frame[imu_frame_idx++] = byte;
        return;
    }

    if (imu_frame_idx == 1)
    {
        if (byte != IMU_PACKET_ACC && byte != IMU_PACKET_GYRO && byte != IMU_PACKET_EULER)
        {
            imu_data.bad_type_count++;
            imu_frame_idx = 0;
            return;
        }

        imu_frame[imu_frame_idx++] = byte;
        return;
    }

    imu_frame[imu_frame_idx++] = byte;

    if (imu_frame_idx >= IMU_FRAME_LEN)
    {
        imu_parse_frame(imu_frame);
        imu_frame_idx = 0;
    }
}

void imu_send_cmd(const char *cmd)
{
#ifdef UART_1_INST
    uart_send_string(UART_1_INST, (char *)cmd, strlen(cmd));
#else
    (void)cmd;
#endif
}

const ImuData_t *imu_get_data(void)
{
    return &imu_data;
}

uint8_t imu_is_online(uint32_t now_ms)
{
    if (!imu_data.euler_valid && !imu_data.gyro_valid && !imu_data.acc_valid)
    {
        return 0;
    }

    return (uint32_t)(now_ms - imu_data.last_update_ms) <= IMU_ONLINE_TIMEOUT_MS;
}
