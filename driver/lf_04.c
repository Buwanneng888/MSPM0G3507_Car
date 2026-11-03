#include "lf_04.h"
#include "ti_msp_dl_config.h"

#if !defined(O1_PORT) || !defined(O1_PIN_4_PIN) || \
    !defined(O2_PORT) || !defined(O2_PIN_6_PIN) || \
    !defined(O3_PORT) || !defined(O3_PIN_7_PIN) || \
    !defined(O4_PORT) || !defined(O4_PIN_8_PIN)
#error "LF-04 GPIO macros are missing; check O1/O2/O3/O4 in SysConfig."
#endif

/* 最近一次四路 GPIO 原始电平：bit0=O1、bit3=O4，1 表示引脚为高电平。 */
static volatile uint8_t g_lf_04_raw_bits = 0U;

/* 最近一次黑线位图：bit0=最左、bit3=最右，1 表示检测到黑线。 */
static volatile uint8_t g_lf_04_black_bits = 0U;

#if LF_04_O1_IS_LEFT == 0U
static uint8_t lf_04_reverse_four_bits(uint8_t value)
{
    value &= LF_04_CHANNEL_MASK;
    return (uint8_t)(((value & 0x01U) << 3) |
                     ((value & 0x02U) << 1) |
                     ((value & 0x04U) >> 1) |
                     ((value & 0x08U) >> 3));
}
#endif

static uint8_t lf_04_sample_raw(void)
{
    uint8_t raw_bits = 0U;

    if (DL_GPIO_readPins(O1_PORT, O1_PIN_4_PIN) != 0U) {
        raw_bits |= 0x01U;
    }
    if (DL_GPIO_readPins(O2_PORT, O2_PIN_6_PIN) != 0U) {
        raw_bits |= 0x02U;
    }
    if (DL_GPIO_readPins(O3_PORT, O3_PIN_7_PIN) != 0U) {
        raw_bits |= 0x04U;
    }
    if (DL_GPIO_readPins(O4_PORT, O4_PIN_8_PIN) != 0U) {
        raw_bits |= 0x08U;
    }

    return raw_bits;
}

void lf_04_init(void)
{
    /* GPIO 输入模式由 SYSCFG_DL_init() 根据 empty.syscfg 完成。 */
    g_lf_04_raw_bits = 0U;
    g_lf_04_black_bits = 0U;
    (void)lf_04_read();
}

uint8_t lf_04_read(void)
{
    uint8_t raw_bits = lf_04_sample_raw();
    uint8_t black_bits;

#if LF_04_BLACK_ACTIVE_LEVEL == 0U
    black_bits = (uint8_t)(~raw_bits) & LF_04_CHANNEL_MASK;
#else
    black_bits = raw_bits & LF_04_CHANNEL_MASK;
#endif

#if LF_04_O1_IS_LEFT == 0U
    black_bits = lf_04_reverse_four_bits(black_bits);
#endif

    g_lf_04_raw_bits = raw_bits;
    g_lf_04_black_bits = black_bits;
    return black_bits;
}

uint8_t lf_04_get_raw_bits(void)
{
    return g_lf_04_raw_bits;
}

uint8_t lf_04_get_black_bits(void)
{
    return g_lf_04_black_bits;
}
