/* Licence
 * Author:  Buwannengfeng
 * Version: V0.10
 * Time:    2026/10/06
 */


/* -------------------------------------------------------------------------- */
/* UART1 八路红外循迹模块应用层（当前启用）                                   */
/* -------------------------------------------------------------------------- */

#include "bsp_system.h"
#include "gray_app.h"
#include "red_gray_driver.h"


#define GRAY_DATA_TIMEOUT_MS       500U
#define GRAY_COMMAND_RETRY_MS     1000U

/* x1 到 x8 从左到右；bit0 对应 x1，权重沿用旧项目的转向尺度。 */
static const int8_t gray_weight[RED_GRAY_CHANNEL_COUNT] =
    {0,0,0, -1, 1, 1, 5, 7};

static volatile uint8_t TrackN = 0U;       /* 最新八路黑线位图：bit0=x1，bit7=x8，1=黑线。 */
static uint32_t gray_last_frame_count = 0U; /* 应用层已经处理的最后一帧数字量序号。 */
static uint32_t gray_last_frame_ms = 0U;    /* 最近有效数字量帧到达时刻，单位：ms。 */
static uint32_t gray_last_command_ms = 0U;  /* 最近一次发送模块输出命令的时刻，单位：ms。 */
static uint8_t gray_online = 0U;            /* UART 模块在线状态：1=500ms 内收到过有效帧。 */

volatile float g_line_position_error = 0.0f; /* 循迹误差：负值=线偏左，正值=线偏右。 */
volatile bool gray_noroad_enable = true;     /* 丢线标志：true=位图中没有黑线通道。 */
volatile bool gray_turn_leftangle = true;    /* 兼容旧转角逻辑的方向状态量。 */

static float gray_calculate_position_error(uint8_t black_bits)
{
    int32_t weighted_sum = 0;
    uint8_t black_count = 0U;
    uint8_t channel;

    for (channel = 0U; channel < RED_GRAY_CHANNEL_COUNT; channel++) {
        if ((black_bits & (uint8_t)(1U << channel)) != 0U) {
            weighted_sum += gray_weight[channel];
            black_count++;
        }
    }

    if (black_count == 0U) {
        return 0.0f;
    }

    return (float)weighted_sum / (float)black_count;
}

void gray_init(void)
{
    my_printf(UART_0_INST, "gray init ok\r\n");
    TrackN = 0U;
    gray_last_frame_count = 0U;
    gray_last_frame_ms = uwTick;
    gray_last_command_ms = uwTick;
    gray_online = 0U;
    g_line_position_error = 0.0f;
    gray_noroad_enable = true;

    /* UART1 的 DMA/环形缓冲已由 scheduler_init() 中的 Uart_Init() 初始化。 */
    red_gray_driver_init();
}

void gray_calibrate(void)
{
    red_gray_driver_calibrate();
    gray_last_command_ms = uwTick;
}

float BlackLine_SetTurn(void)
{
    return gray_calculate_position_error(TrackN);
}

void gray_task(void)
{
//		my_printf(UART_0_INST,"gray in/r/n");
	
    const RedGrayData_t *data;
    uint32_t now = uwTick;
		
	//现场校准
    if ((key_down & KEY_MASK_LEFT) != 0U) {
        gray_calibrate();
    }

    red_gray_driver_poll();
    data = red_gray_driver_get_data();

    if ((data->digital_valid != 0U) &&
        (data->digital_frame_count != gray_last_frame_count)) {
        gray_last_frame_count = data->digital_frame_count;
        gray_last_frame_ms = now;
        gray_online = 1U;

        /* 驱动已将模块的 0=黑线转换成项目兼容的 1=黑线位图。 */
        TrackN = red_gray_driver_get_black_bits();
				//打印数字量
//				my_printf(UART_0_INST,"{Digital}%d,%d,%d,%d,%d,%d,%d,%d\r\n",
//										(TrackN >> 0) & 0x01,(TrackN >> 1) & 0x01,
//										(TrackN >> 2) & 0x01,(TrackN >> 3) & 0x01,
//										(TrackN >> 4) & 0x01,(TrackN >> 5) & 0x01,
//										(TrackN >> 6) & 0x01,(TrackN >> 7) & 0x01);
								
        gray_noroad_enable = (TrackN == 0U);
        g_line_position_error = BlackLine_SetTurn();
    }

    if ((gray_online == 0U) ||
        ((uint32_t)(now - gray_last_frame_ms) > GRAY_DATA_TIMEOUT_MS)) {
        gray_online = 0U;
        TrackN = 0U;
        gray_noroad_enable = true;

        /* 模块可能晚于主控上电；离线时周期重发数字量输出命令。 */
        if ((uint32_t)(now - gray_last_command_ms) >= GRAY_COMMAND_RETRY_MS) {
            red_gray_driver_set_output(0U, 0U, 1U);
            gray_last_command_ms = now;
        }
    }
}

uint8_t gray_get_digital(void)
{
    return TrackN;
}

uint8_t gray_is_online(void)
{
    return gray_online;
}








