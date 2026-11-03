/* Licence
 * Author:  Buwannengfeng
 * Version: V0.10
 * Time:    2026/10/06
 */

#include "oled_app.h"

/*
 * oled_app：演示用显示任务。
 * 默认只显示基础状态；下面各段演示代码按需取消注释，一次留一段即可（屏幕 4 行）。
 */

extern int basic_speed;
extern volatile bool pid_running;

static void oled_make_gray_string(char *buffer, uint8_t digital)
{
    uint8_t i;

    for (i = 0U; i < GRAY_CHANNEL_COUNT; i++) {
        buffer[i] = ((digital >> i) & 0x01U) != 0U ? '1' : '0';
    }
    buffer[GRAY_CHANNEL_COUNT] = '\0';
}

void oled_app_init(void)
{
    my_printf(UART_0_INST, "oled init ok\r\n");
    OLED_Init();
    OLED_Clear();
}

void oled_app_task(void)
{
    char gray_bits[GRAY_CHANNEL_COUNT + 1U];
    char line[17];

    /* ---------------- 段 1：基础状态（默认启用） ---------------- */
    /* 第 1 行：当前按下的按键名 */
    snprintf(line, sizeof(line), "KEY:%-6s", key_get_active_name());
    OLED_ShowString(1, 1, line);

    /* 第 2 行：八路灰度位图，bit0=x1（最左），1=黑线 */
    oled_make_gray_string(gray_bits, gray_get_digital());
    snprintf(line, sizeof(line), "GRAY:%s", gray_bits);
    OLED_ShowString(2, 1, line);

    /* 第 3 行：左右编码器实测速度，单位 cm/s */
    snprintf(line, sizeof(line), "L%4.0f R%4.0f",
             (double)encoder_left.speed_cm_s, (double)encoder_right.speed_cm_s);
    OLED_ShowString(3, 1, line);

    /* 第 4 行：灰度模块在线状态 + PID 总开关 */
    snprintf(line, sizeof(line), "G:%s PID:%s",
             gray_is_online() ? "OK " : "OFF", pid_running ? "ON " : "OFF");
    OLED_ShowString(4, 1, line);

    /* ---------------- 段 2：速度环（PID 打开时看这行） ---------------- */
    // snprintf(line, sizeof(line), "TGT:%d   ", basic_speed);
    // OLED_ShowString(1, 1, line);
    // snprintf(line, sizeof(line), "L%.1f R%.1f",
    //          (double)encoder_left.speed_cm_s, (double)encoder_right.speed_cm_s);
    // OLED_ShowString(2, 1, line);
    // snprintf(line, sizeof(line), "LINE_ERR:%.2f", (double)g_line_position_error);
    // OLED_ShowString(3, 1, line);

    /* ---------------- 段 3：电机原始输出（调电机方向用） ---------------- */
    // snprintf(line, sizeof(line), "left :%d   ", left_motor.speed);
    // OLED_ShowString(1, 1, line);
    // snprintf(line, sizeof(line), "right:%d   ", right_motor.speed);
    // OLED_ShowString(2, 1, line);

    /* ---------------- 段 4：IMU 姿态（需先在 scheduler 打开 imu_imu_task） ---------------- */
    // snprintf(line, sizeof(line), "Roll :%.1f", (double)imu_imu_get_roll());
    // OLED_ShowString(1, 1, line);
    // snprintf(line, sizeof(line), "Pitch:%.1f", (double)imu_imu_get_pitch());
    // OLED_ShowString(2, 1, line);
    // snprintf(line, sizeof(line), "Yaw  :%.1f", (double)imu_imu_get_yaw());
    // OLED_ShowString(3, 1, line);
}
