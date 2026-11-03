#ifndef __RED_GRAY_DRIVER_H__
#define __RED_GRAY_DRIVER_H__

#include <stdint.h>

#define RED_GRAY_CHANNEL_COUNT 8U

/**
 * @brief 八路红外循迹模块的最新数据及接收统计。
 *
 * digital[] 保留模块原始电平：0 表示黑线，1 表示白底。
 * 应用层如需“1 表示黑线”的位图，应调用
 * red_gray_driver_get_black_bits()。
 */
typedef struct
{
    uint8_t digital[RED_GRAY_CHANNEL_COUNT];  /**< 八路原始数字电平：0=黑线，1=白底。 */
    uint16_t analog[RED_GRAY_CHANNEL_COUNT];  /**< 八路模块模拟量，数值范围由模块协议决定。 */
    uint32_t digital_frame_count;             /**< 已成功解析的数字量帧累计数。 */
    uint32_t analog_frame_count;              /**< 已成功解析的模拟量帧累计数。 */
    uint32_t invalid_frame_count;             /**< 格式、长度或内容错误的帧累计数。 */
    uint8_t digital_valid;                    /**< 数字量有效标志：1=至少收到过一帧有效数据。 */
    uint8_t analog_valid;                     /**< 模拟量有效标志：1=至少收到过一帧有效数据。 */
} RedGrayData_t;

/** 初始化协议解析器，并通过 UART1 开启数字量连续输出。 */
void red_gray_driver_init(void);

/** 从工程现有的 UART1 DMA 环形缓冲区取数并解析完整的 $...# 帧。 */
void red_gray_driver_poll(void);

/**
 * 发送模块控制命令：$校准,模拟量,数字量#。
 * 每个参数仅使用最低位，非 0 值均按 1 发送。
 */
void red_gray_driver_set_output(uint8_t calibrate,
                                uint8_t analog_enable,
                                uint8_t digital_enable);

/** 触发模块自身校准，并保持数字量输出开启。 */
void red_gray_driver_calibrate(void);

/** 返回驱动内部最新数据的只读指针。 */
const RedGrayData_t *red_gray_driver_get_data(void);

/**
 * 返回兼容旧应用层的黑线位图：bit0=x1(最左)，bit7=x8(最右)，
 * 置 1 表示对应探头检测到黑线。
 */
uint8_t red_gray_driver_get_black_bits(void);

#endif
