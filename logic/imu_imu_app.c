/* Licence
 * Author:  Buwannengfeng
 * Version: V0.10
 * Time:    2026/10/06
 */

#include <math.h>
#include <string.h>
#include "imu_imu_app.h"
#include "icm42688_driver.h"

extern volatile uint32_t uwTick;

#define M_PI_F                         (3.14159265358979323846f)
#define IMU_SAMPLE_PERIOD_MS           (10U)
#define IMU_ONLINE_TIMEOUT_MS          (100U)
#define IMU_REINIT_INTERVAL_MS         (500U)
#define IMU_MAX_CONSECUTIVE_ERRORS     (5U)

#define GYRO_CAL_WINDOW                (100U)
#define GYRO_CAL_VAR_THRESHOLD         (0.05f)
#define ACC_STATIC_MIN_MG              (850.0f)
#define ACC_STATIC_MAX_MG              (1150.0f)
#define BIAS_REFRESH_INTERVAL_MS       (1000U)
#define BIAS_REFRESH_ALPHA             (0.10f)
#define GYRO_INITIAL_MEAN_MAX_DPS       (5.0f)
#define GYRO_STATIC_RESIDUAL_MAX_DPS    (2.0f)

#define AHRS_KP_INITIAL                (6.0f)
#define AHRS_KP_STEADY                 (0.5f)
#define AHRS_KI                        (0.001f)
#define AHRS_FAST_CONVERGE_MS          (1500U)

imu_imu_data_t imu_imu;

static float q0 = 1.0f;
static float q1 = 0.0f;
static float q2 = 0.0f;
static float q3 = 0.0f;
static float ex_integral = 0.0f;
static float ey_integral = 0.0f;
static float ez_integral = 0.0f;

static float gyro_history[3][GYRO_CAL_WINDOW];
static float gyro_sum[3];
static float gyro_square_sum[3];
static uint16_t gyro_history_count;
static uint16_t gyro_history_index;
static uint32_t last_bias_refresh_ms;
static uint8_t consecutive_read_errors;
static uint32_t last_reinit_attempt_ms;

static float inv_sqrt(float value)
{
    union {
        float f;
        uint32_t u;
    } data;
    float half;

    if (value <= 0.0f) {
        return 0.0f;
    }

    half = value * 0.5f;
    data.f = value;
    data.u = 0x5F3759DFU - (data.u >> 1);
    data.f = data.f * (1.5f - half * data.f * data.f);
    return data.f;
}

static void reset_attitude_filter(uint32_t now_ms)
{
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    ex_integral = 0.0f;
    ey_integral = 0.0f;
    ez_integral = 0.0f;

    imu_imu.quat[0] = 1.0f;
    imu_imu.quat[1] = 0.0f;
    imu_imu.quat[2] = 0.0f;
    imu_imu.quat[3] = 0.0f;
    imu_imu.euler.x = 0.0f;
    imu_imu.euler.y = 0.0f;
    imu_imu.euler.z = 0.0f;
    imu_imu.last_update_ms = now_ms;
}

static void reset_gyro_calibration(void)
{
    memset(gyro_history, 0, sizeof(gyro_history));
    memset(gyro_sum, 0, sizeof(gyro_sum));
    memset(gyro_square_sum, 0, sizeof(gyro_square_sum));
    gyro_history_count = 0U;
    gyro_history_index = 0U;
    last_bias_refresh_ms = 0U;

    imu_imu.gyro_offset.x = 0.0f;
    imu_imu.gyro_offset.y = 0.0f;
    imu_imu.gyro_offset.z = 0.0f;
    imu_imu.gyro_z_nobias = 0.0f;
    imu_imu.calibrated = 0U;
    imu_imu.ready = 0U;
    imu_imu.state = IMU_IMU_STATE_CALIBRATING;
}

static void gyro_window_push(float gx, float gy, float gz)
{
    float input[3];
    uint8_t axis;

    input[0] = gx;
    input[1] = gy;
    input[2] = gz;

    for (axis = 0U; axis < 3U; axis++) {
        if (gyro_history_count >= GYRO_CAL_WINDOW) {
            float old = gyro_history[axis][gyro_history_index];
            gyro_sum[axis] -= old;
            gyro_square_sum[axis] -= old * old;
        }

        gyro_history[axis][gyro_history_index] = input[axis];
        gyro_sum[axis] += input[axis];
        gyro_square_sum[axis] += input[axis] * input[axis];
    }

    gyro_history_index++;
    if (gyro_history_index >= GYRO_CAL_WINDOW) {
        gyro_history_index = 0U;
    }
    if (gyro_history_count < GYRO_CAL_WINDOW) {
        gyro_history_count++;
    }
}

static uint8_t sensor_is_static(const icm42688_real_data_t *acc,
                                float average[3])
{
    float variance[3];
    float count;
    float acc_norm_square;
    float min_norm_square;
    float max_norm_square;
    uint8_t axis;

    if ((acc == 0) || (average == 0) ||
        (gyro_history_count < GYRO_CAL_WINDOW)) {
        return 0U;
    }

    count = (float)GYRO_CAL_WINDOW;
    for (axis = 0U; axis < 3U; axis++) {
        float reference;
        float mean_error;

        average[axis] = gyro_sum[axis] / count;
        variance[axis] = gyro_square_sum[axis] / count - average[axis] * average[axis];
        if (variance[axis] < 0.0f) {
            variance[axis] = 0.0f;
        }
        if (variance[axis] > GYRO_CAL_VAR_THRESHOLD) {
            return 0U;
        }

        reference = (axis == 0U) ? imu_imu.gyro_offset.x :
                    ((axis == 1U) ? imu_imu.gyro_offset.y : imu_imu.gyro_offset.z);
        mean_error = fabsf(average[axis] - reference);
        if (imu_imu.calibrated == 0U) {
            if (fabsf(average[axis]) > GYRO_INITIAL_MEAN_MAX_DPS) {
                return 0U;
            }
        } else if (mean_error > GYRO_STATIC_RESIDUAL_MAX_DPS) {
            return 0U;
        }
    }

    acc_norm_square = acc->x * acc->x + acc->y * acc->y + acc->z * acc->z;
    min_norm_square = ACC_STATIC_MIN_MG * ACC_STATIC_MIN_MG;
    max_norm_square = ACC_STATIC_MAX_MG * ACC_STATIC_MAX_MG;

    return ((acc_norm_square >= min_norm_square) &&
            (acc_norm_square <= max_norm_square)) ? 1U : 0U;
}

static void update_gyro_bias(const icm42688_real_data_t *acc, uint32_t now_ms)
{
    float average[3];

    if (sensor_is_static(acc, average) == 0U) {
        return;
    }

    if (imu_imu.calibrated == 0U) {
        imu_imu.gyro_offset.x = average[0];
        imu_imu.gyro_offset.y = average[1];
        imu_imu.gyro_offset.z = average[2];
        imu_imu.calibrated = 1U;
        imu_imu.ready = 1U;
        imu_imu.state = IMU_IMU_STATE_READY;
        imu_imu.init_time_ms = now_ms;
        imu_imu.kp = AHRS_KP_INITIAL;
        last_bias_refresh_ms = now_ms;
        reset_attitude_filter(now_ms);
        return;
    }

    if ((uint32_t)(now_ms - last_bias_refresh_ms) >= BIAS_REFRESH_INTERVAL_MS) {
        imu_imu.gyro_offset.x += (average[0] - imu_imu.gyro_offset.x) * BIAS_REFRESH_ALPHA;
        imu_imu.gyro_offset.y += (average[1] - imu_imu.gyro_offset.y) * BIAS_REFRESH_ALPHA;
        imu_imu.gyro_offset.z += (average[2] - imu_imu.gyro_offset.z) * BIAS_REFRESH_ALPHA;
        last_bias_refresh_ms = now_ms;
    }
}

static void ahrs_update(float gx, float gy, float gz,
                        float ax, float ay, float az,
                        uint32_t now_ms)
{
    uint32_t delta_ms;
    float dt;
    float half_dt;
    float norm;
    float vx;
    float vy;
    float vz;
    float ex;
    float ey;
    float ez;
    float next_q0;
    float next_q1;
    float next_q2;
    float next_q3;

    delta_ms = (uint32_t)(now_ms - imu_imu.last_update_ms);
    if (delta_ms == 0U) {
        return;
    }
    if (delta_ms > 50U) {
        delta_ms = 50U;
    }
    dt = (float)delta_ms * 0.001f;
    half_dt = 0.5f * dt;
    imu_imu.last_update_ms = now_ms;

    norm = inv_sqrt(ax * ax + ay * ay + az * az);
    if (norm == 0.0f) {
        return;
    }
    ax *= norm;
    ay *= norm;
    az *= norm;

    vx = 2.0f * (q1 * q3 - q0 * q2);
    vy = 2.0f * (q0 * q1 + q2 * q3);
    vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

    ex = ay * vz - az * vy;
    ey = az * vx - ax * vz;
    ez = ax * vy - ay * vx;

    ex_integral += ex * AHRS_KI * dt;
    ey_integral += ey * AHRS_KI * dt;
    ez_integral += ez * AHRS_KI * dt;

    gx += imu_imu.kp * ex + ex_integral;
    gy += imu_imu.kp * ey + ey_integral;
    gz += imu_imu.kp * ez + ez_integral;

    next_q0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * half_dt;
    next_q1 = q1 + ( q0 * gx + q2 * gz - q3 * gy) * half_dt;
    next_q2 = q2 + ( q0 * gy - q1 * gz + q3 * gx) * half_dt;
    next_q3 = q3 + ( q0 * gz + q1 * gy - q2 * gx) * half_dt;

    norm = inv_sqrt(next_q0 * next_q0 + next_q1 * next_q1 +
                    next_q2 * next_q2 + next_q3 * next_q3);
    if (norm == 0.0f) {
        return;
    }

    q0 = next_q0 * norm;
    q1 = next_q1 * norm;
    q2 = next_q2 * norm;
    q3 = next_q3 * norm;

    imu_imu.quat[0] = q0;
    imu_imu.quat[1] = q1;
    imu_imu.quat[2] = q2;
    imu_imu.quat[3] = q3;
}

static void quaternion_to_euler(void)
{
    float pitch_input;

    pitch_input = -2.0f * q1 * q3 + 2.0f * q0 * q2;
    if (pitch_input > 1.0f) {
        pitch_input = 1.0f;
    } else if (pitch_input < -1.0f) {
        pitch_input = -1.0f;
    }

    /* Keep the original project's yaw sign and axis convention. */
    imu_imu.euler.x = -atan2f(2.0f * q1 * q2 + 2.0f * q0 * q3,
                              1.0f - 2.0f * q2 * q2 - 2.0f * q3 * q3) *
                      180.0f / M_PI_F;
    imu_imu.euler.y = -asinf(pitch_input) * 180.0f / M_PI_F;
    imu_imu.euler.z = atan2f(2.0f * q2 * q3 + 2.0f * q0 * q1,
                             1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2) *
                     180.0f / M_PI_F;
}

static void mark_sensor_offline(uint32_t now_ms)
{
    imu_imu.sensor_ok = 0U;
    imu_imu.ready = 0U;
    imu_imu.calibrated = 0U;
    imu_imu.state = IMU_IMU_STATE_OFFLINE;
    last_reinit_attempt_ms = now_ms;
    consecutive_read_errors = 0U;
    reset_gyro_calibration();
    imu_imu.state = IMU_IMU_STATE_OFFLINE;
}

static uint8_t try_sensor_init(uint32_t now_ms)
{
    if (icm42688_init() != ICM42688_OK) {
        imu_imu.sensor_ok = 0U;
        imu_imu.ready = 0U;
        imu_imu.state = IMU_IMU_STATE_OFFLINE;
        last_reinit_attempt_ms = now_ms;
        return 0U;
    }

    now_ms = uwTick;
    imu_imu.sensor_ok = 1U;
    imu_imu.who_am_i = icm42688_get_who_am_i();
    imu_imu.i2c_address = icm42688_get_i2c_address();
    imu_imu.last_sample_ms = now_ms;
    imu_imu.last_good_ms = now_ms;
    consecutive_read_errors = 0U;
    reset_gyro_calibration();
    reset_attitude_filter(now_ms);
    return 1U;
}

void imu_imu_init(void)
{
    uint32_t now_ms = uwTick;

    memset(&imu_imu, 0, sizeof(imu_imu));
    reset_attitude_filter(now_ms);
    reset_gyro_calibration();
    imu_imu.state = IMU_IMU_STATE_OFFLINE;
    imu_imu.kp = AHRS_KP_INITIAL;
    last_reinit_attempt_ms = now_ms;
    consecutive_read_errors = 0U;

    (void)try_sensor_init(now_ms);
}

void imu_imu_task(void)
{
    uint32_t now_ms = uwTick;
    icm42688_real_data_t acc;
    icm42688_real_data_t gyro;
    int8_t status;
    float gx;
    float gy;
    float gz;

    if (imu_imu.sensor_ok == 0U) {
        if ((uint32_t)(now_ms - last_reinit_attempt_ms) >= IMU_REINIT_INTERVAL_MS) {
            imu_imu.reinit_count++;
            last_reinit_attempt_ms = now_ms;
            (void)try_sensor_init(now_ms);
        }
        return;
    }

    if ((uint32_t)(now_ms - imu_imu.last_sample_ms) < IMU_SAMPLE_PERIOD_MS) {
        return;
    }
    imu_imu.last_sample_ms = now_ms;

    status = icm42688_get_raw_data(&acc, &gyro);
    if (status != ICM42688_OK) {
        imu_imu.read_error_count++;
        consecutive_read_errors++;
        if (consecutive_read_errors >= IMU_MAX_CONSECUTIVE_ERRORS) {
            mark_sensor_offline(now_ms);
        }
        return;
    }

    consecutive_read_errors = 0U;
    imu_imu.last_good_ms = now_ms;
    imu_imu.sample_count++;

    imu_imu.raw.acc.x = acc.x;
    imu_imu.raw.acc.y = acc.y;
    imu_imu.raw.acc.z = acc.z;
    imu_imu.raw.gyro.x = gyro.x;
    imu_imu.raw.gyro.y = gyro.y;
    imu_imu.raw.gyro.z = gyro.z;

    gyro_window_push(gyro.x, gyro.y, gyro.z);
    update_gyro_bias(&acc, now_ms);

    if (imu_imu.calibrated == 0U) {
        imu_imu.ready = 0U;
        imu_imu.state = IMU_IMU_STATE_CALIBRATING;
        return;
    }

    if ((uint32_t)(now_ms - imu_imu.init_time_ms) >= AHRS_FAST_CONVERGE_MS) {
        imu_imu.kp = AHRS_KP_STEADY;
    }

    gx = (gyro.x - imu_imu.gyro_offset.x) * M_PI_F / 180.0f;
    gy = (gyro.y - imu_imu.gyro_offset.y) * M_PI_F / 180.0f;
    gz = (gyro.z - imu_imu.gyro_offset.z) * M_PI_F / 180.0f;
    imu_imu.gyro_z_nobias = gyro.z - imu_imu.gyro_offset.z;

    ahrs_update(gx, gy, gz, acc.x, acc.y, acc.z, now_ms);
    quaternion_to_euler();

    imu_imu.ready = 1U;
    imu_imu.state = IMU_IMU_STATE_READY;
//		my_printf(UART_0_INST, "x,y,z:%.2f,%.2f,%.2f\r\n",imu_imu.gyro_offset.x,imu_imu.gyro_offset.y,imu_imu.gyro_offset.z);
}

const imu_imu_data_t *imu_imu_get_data(void)
{
    return &imu_imu;
}

float imu_imu_get_yaw(void)
{
    return imu_imu.euler.x;
}

float imu_imu_get_gyro_z(void)
{
    return imu_imu.gyro_z_nobias;
}

uint8_t imu_imu_app_is_ready(void)
{
    if ((imu_imu.ready == 0U) || (imu_imu.sensor_ok == 0U) ||
        (imu_imu.calibrated == 0U)) {
        return 0U;
    }

    return ((uint32_t)(uwTick - imu_imu.last_good_ms) <= IMU_ONLINE_TIMEOUT_MS) ? 1U : 0U;
}
