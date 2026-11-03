/* Licence
 * Author:  Buwannengfeng
 * Version: V0.10
 * Time:    2026/10/06
 */

#include "bsp_system.h"
#include "imu_app.h"

#define IMU_INIT_START_DELAY_MS 200
#define IMU_INIT_CMD_INTERVAL_MS 30
#define IMU_INIT_STABLE_DELAY_MS 500
#define IMU_INIT_READY_TIMEOUT_MS 4000
#define IMU_DEBUG_PRINT 0

typedef enum
{
    IMU_APP_INIT_BUSY = 0,
    IMU_APP_INIT_DONE
} ImuAppInitState_t;

static ImuAppInitState_t imu_init_state;
static uint32_t imu_debug_last_print;
static float imu_yaw_zero;
static uint8_t imu_ready;

static float imu_normalize_angle(float angle)
{
    while (angle > 180.0f)
    {
        angle -= 360.0f;
    }

    while (angle < -180.0f)
    {
        angle += 360.0f;
    }

    return angle;
}

static void imu_app_send_cmd_with_delay(const char *cmd)
{
    imu_send_cmd(cmd);
    DL_Delay(IMU_INIT_CMD_INTERVAL_MS);
}

void imu_app_init(void)
{
    const ImuData_t *imu;
    uint32_t wait_start;
    uint32_t reset_frame_count;

    imu_driver_init();
    imu_init_state = IMU_APP_INIT_BUSY;
    imu_debug_last_print = 0;
    imu_yaw_zero = 0.0f;
    imu_ready = 0;

#if IMU_DEBUG_PRINT
    my_printf(UART_0_INST, "imu init\r\n");
#endif

    DL_Delay(IMU_INIT_START_DELAY_MS);
    imu_app_send_cmd_with_delay("AT+HEX\r\n");
    imu_app_send_cmd_with_delay("AT+TIM=20\r\n");
    imu_app_send_cmd_with_delay("AT+ACC=000\r\n");
    imu_app_send_cmd_with_delay("AT+GYRO=001\r\n");
    imu_app_send_cmd_with_delay("AT+EULA=001\r\n");
    imu_app_send_cmd_with_delay("AT+RST\r\n");
    reset_frame_count = imu_get_data()->frame_count;

    DL_Delay(IMU_INIT_STABLE_DELAY_MS);

    imu = imu_get_data();
    wait_start = uwTick;
    while ((uint32_t)(uwTick - wait_start) < IMU_INIT_READY_TIMEOUT_MS)
    {
        if (imu->frame_count > reset_frame_count &&
            imu->euler_valid &&
            imu->gyro_valid &&
            imu_is_online(uwTick))
        {
            imu_ready = 1;
            break;
        }
    }

    imu_init_state = IMU_APP_INIT_DONE;
}

void imu_app_task(void)
{
    const ImuData_t *imu = imu_get_data();

    if (imu_init_state == IMU_APP_INIT_DONE)
    {
        imu_ready = imu->euler_valid && imu->gyro_valid && imu_is_online(uwTick);
    }

#if IMU_DEBUG_PRINT
    if ((uint32_t)(uwTick - imu_debug_last_print) >= 200)
    {
        imu_debug_last_print = uwTick;
        my_printf(UART_0_INST,
                  "imu s:%d r:%d o:%d rx:%lu f:%lu e:%d g:%d y:%.1f z:%.1f c:%lu h:%lu t:%lu\r\n",
                  imu_init_state,
                  imu_ready,
                  imu_is_online(uwTick),
                  (unsigned long)imu->rx_count,
                  (unsigned long)imu->frame_count,
                  imu->euler_valid,
                  imu->gyro_valid,
                  imu_get_yaw(),
                  imu_get_gyro_z(),
                  (unsigned long)imu->checksum_error,
                  (unsigned long)imu->bad_header_count,
                  (unsigned long)imu->bad_type_count);
    }
#endif
}

void imu_zero_yaw(void)
{
    imu_yaw_zero = imu_get_data()->yaw;
}

float imu_get_yaw(void)
{
    return imu_normalize_angle(imu_get_data()->yaw - imu_yaw_zero);
}

float imu_get_yaw_error(float target_yaw)
{
    return imu_normalize_angle(target_yaw - imu_get_yaw());
}

float imu_get_gyro_z(void)
{
    return imu_get_data()->gyro_z;
}

uint8_t imu_app_is_ready(void)
{
    return imu_ready;
}
