#ifndef __LF_04_H
#define __LF_04_H

#include <stdint.h>

/*
 * WHEELTEC LF-04 四路红外循迹模块。
 *
 * 接线：O1 -> PA31，O2 -> PA12，O3 -> PB8，O4 -> PB27。
 * gray_app 使用 bit0..bit3 表示从左到右的四个探头，置 1 表示黑线。
 * 若实车模块方向或输出极性相反，只需修改下面两个配置宏。
 */
 //该部分为oled显示更换
#define LF_04_CHANNEL_COUNT          4U     /* 模块探头数量。 */
#define LF_04_CHANNEL_MASK           0x0FU  /* 有效黑线位图掩码：bit0~bit3。 */

/* LF-04 比较器在黑线上默认输出低电平。 */
#define LF_04_BLACK_ACTIVE_LEVEL     0U     /* 0=黑线低有效；1=黑线高有效。 */

/* 默认认为 O1 是车头朝前时的最左探头，O4 是最右探头。 */
#define LF_04_O1_IS_LEFT             1U     /* 1=O1 在最左；0=O1 在最右并自动反转位序。 */

/** 初始化 LF-04 软件状态并立即完成一次 GPIO 采样。 */
void lf_04_init(void);

/** 采样四路输入并返回黑线位图：bit0=最左，bit3=最右，1=黑线。 */
uint8_t lf_04_read(void);

/** 返回最近一次采样的引脚原始电平：bit0=O1，bit3=O4，1=高电平。 */
uint8_t lf_04_get_raw_bits(void);

/** 返回最近一次采样的黑线位图：bit0=最左，bit3=最右。 */
uint8_t lf_04_get_black_bits(void);

#endif
