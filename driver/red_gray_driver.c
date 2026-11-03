#include "red_gray_driver.h"

#include <string.h>

#include "ti_msp_dl_config.h"
#include "uart_driver.h"

#define RED_GRAY_FRAME_BUFFER_SIZE 96U /* 单个协议帧缓存容量，单位：字节。 */
#define RED_GRAY_UART_READ_SIZE    64U /* 每次从 UART1 环形缓冲读取的最大字节数。 */

static RedGrayData_t red_gray_data;                         /* 最新有效数据与帧统计状态。 */
static char red_gray_frame[RED_GRAY_FRAME_BUFFER_SIZE];     /* 当前正在拼接的 $...# 协议帧。 */
static uint16_t red_gray_frame_length;                      /* 当前帧已接收长度，单位：字节。 */
static uint8_t red_gray_receiving;                          /* 接收状态：1=已遇到 '$'，正在等待 '#'。 */

static uint8_t red_gray_expect_channel(const char **cursor, uint8_t channel)
{
    const char *p = *cursor;

    if ((p[0] != 'x') ||
        (p[1] != (char)('1' + channel)) ||
        (p[2] != ':')) {
        return 0U;
    }

    *cursor = p + 3;
    return 1U;
}

static uint8_t red_gray_parse_digital(const char *frame)
{
    uint8_t parsed[RED_GRAY_CHANNEL_COUNT];
    const char *cursor = frame;
    uint8_t channel;

    if ((cursor[0] != '$') || (cursor[1] != 'D') || (cursor[2] != ',')) {
        return 0U;
    }
    cursor += 3;

    for (channel = 0U; channel < RED_GRAY_CHANNEL_COUNT; channel++) {
        char delimiter = (channel + 1U < RED_GRAY_CHANNEL_COUNT) ? ',' : '#';

        if (red_gray_expect_channel(&cursor, channel) == 0U) {
            return 0U;
        }
        if ((cursor[0] != '0') && (cursor[0] != '1')) {
            return 0U;
        }

        parsed[channel] = (uint8_t)(cursor[0] - '0');
        cursor++;

        if (cursor[0] != delimiter) {
            return 0U;
        }
        cursor++;
    }

    if (cursor[0] != '\0') {
        return 0U;
    }

    memcpy(red_gray_data.digital, parsed, sizeof(parsed));
    red_gray_data.digital_valid = 1U;
    red_gray_data.digital_frame_count++;
    return 1U;
}

static uint8_t red_gray_parse_u16(const char **cursor, uint16_t *value)
{
    const char *p = *cursor;
    uint32_t result = 0U;

    if ((p[0] < '0') || (p[0] > '9')) {
        return 0U;
    }

    while ((p[0] >= '0') && (p[0] <= '9')) {
        result = result * 10U + (uint32_t)(p[0] - '0');
        if (result > 65535U) {
            return 0U;
        }
        p++;
    }

    *value = (uint16_t)result;
    *cursor = p;
    return 1U;
}

static uint8_t red_gray_parse_analog(const char *frame)
{
    uint16_t parsed[RED_GRAY_CHANNEL_COUNT];
    const char *cursor = frame;
    uint8_t channel;

    if ((cursor[0] != '$') || (cursor[1] != 'A') || (cursor[2] != ',')) {
        return 0U;
    }
    cursor += 3;

    for (channel = 0U; channel < RED_GRAY_CHANNEL_COUNT; channel++) {
        char delimiter = (channel + 1U < RED_GRAY_CHANNEL_COUNT) ? ',' : '#';

        if (red_gray_expect_channel(&cursor, channel) == 0U) {
            return 0U;
        }
        if (red_gray_parse_u16(&cursor, &parsed[channel]) == 0U) {
            return 0U;
        }
        if (cursor[0] != delimiter) {
            return 0U;
        }
        cursor++;
    }

    if (cursor[0] != '\0') {
        return 0U;
    }

    memcpy(red_gray_data.analog, parsed, sizeof(parsed));
    red_gray_data.analog_valid = 1U;
    red_gray_data.analog_frame_count++;
    return 1U;
}

static void red_gray_process_frame(const char *frame)
{
    uint8_t valid = 0U;

    if (frame[1] == 'D') {
        valid = red_gray_parse_digital(frame);
    }
    else if (frame[1] == 'A') {
        valid = red_gray_parse_analog(frame);
    }

    if (valid == 0U) {
        red_gray_data.invalid_frame_count++;
    }
}

static void red_gray_feed_byte(uint8_t byte)
{
    if (byte == (uint8_t)'$') {
        red_gray_frame_length = 0U;
        red_gray_receiving = 1U;
    }

    if (red_gray_receiving == 0U) {
        return;
    }

    if (red_gray_frame_length >= (RED_GRAY_FRAME_BUFFER_SIZE - 1U)) {
        red_gray_frame_length = 0U;
        red_gray_receiving = 0U;
        red_gray_data.invalid_frame_count++;
        return;
    }

    red_gray_frame[red_gray_frame_length++] = (char)byte;

    if (byte == (uint8_t)'#') {
        red_gray_frame[red_gray_frame_length] = '\0';
        red_gray_process_frame(red_gray_frame);
        red_gray_frame_length = 0U;
        red_gray_receiving = 0U;
    }
}

void red_gray_driver_set_output(uint8_t calibrate,
                                uint8_t analog_enable,
                                uint8_t digital_enable)
{
#ifdef UART_1_INST
    char command[8] = "$0,0,0#";

    command[1] = (calibrate != 0U) ? '1' : '0';
    command[3] = (analog_enable != 0U) ? '1' : '0';
    command[5] = (digital_enable != 0U) ? '1' : '0';
    uart_send_string(UART_1_INST, command, 7);
#else
    (void)calibrate;
    (void)analog_enable;
    (void)digital_enable;
#endif
}

void red_gray_driver_init(void)
{
    uint8_t discard[RED_GRAY_UART_READ_SIZE];

    memset(&red_gray_data, 0, sizeof(red_gray_data));
    memset(red_gray_frame, 0, sizeof(red_gray_frame));
    red_gray_frame_length = 0U;
    red_gray_receiving = 0U;

    /* Uart_Init() 已完成 UART1 DMA 初始化；先丢弃此前可能残留的半帧。 */
    Uart1_Rx_Update();
    while (Uart1_Available() > 0U) {
        rt_size_t available = Uart1_Available();
        uint16_t read_size = (available > RED_GRAY_UART_READ_SIZE) ?
                             RED_GRAY_UART_READ_SIZE : (uint16_t)available;
        if (Uart1_Read(discard, read_size) == 0U) {
            break;
        }
    }

    /* 与移植例程一致：关闭模拟量，仅连续发送八路数字量。 */
    red_gray_driver_set_output(0U, 0U, 1U);
}

void red_gray_driver_poll(void)
{
    uint8_t rx_data[RED_GRAY_UART_READ_SIZE];

    Uart1_Rx_Update();

    while (Uart1_Available() > 0U) {
        rt_size_t available = Uart1_Available();
        uint16_t read_size = (available > RED_GRAY_UART_READ_SIZE) ?
                             RED_GRAY_UART_READ_SIZE : (uint16_t)available;
        rt_size_t actual = Uart1_Read(rx_data, read_size);
        rt_size_t i;

        if (actual == 0U) {
            break;
        }

        for (i = 0U; i < actual; i++) {
            red_gray_feed_byte(rx_data[i]);
        }
    }
}

void red_gray_driver_calibrate(void)
{
    red_gray_driver_set_output(1U, 0U, 1U);
}

const RedGrayData_t *red_gray_driver_get_data(void)
{
    return &red_gray_data;
}

uint8_t red_gray_driver_get_black_bits(void)
{
    uint8_t bits = 0U;
    uint8_t channel;

    if (red_gray_data.digital_valid == 0U) {
        return 0U;
    }

    for (channel = 0U; channel < RED_GRAY_CHANNEL_COUNT; channel++) {
        /* 模块原始值 0=黑线；项目旧接口约定置 1=黑线。 */
        if (red_gray_data.digital[channel] == 0U) {
            bits |= (uint8_t)(1U << channel);
        }
    }

    return bits;
}
