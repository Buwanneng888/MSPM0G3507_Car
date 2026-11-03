#ifndef __BUTTON_DRIVER_H
#define __BUTTON_DRIVER_H

#include <stdint.h>

typedef enum {
    KEY_MASK_NONE      = 0x00,
    KEY_MASK_MY_UP     = 0x01,
    KEY_MASK_MY_DOWN   = 0x02,
    KEY_MASK_LEFT      = 0x04,
    KEY_MASK_RIGHT     = 0x08,
    KEY_MASK_CENTER    = 0x10,
} key_mask_t;

void key_task(void);
uint8_t key_read(void);
const char *key_get_active_name(void);
const char *key_get_last_pressed_name(void);

extern uint8_t key_val;				//正在被按下
extern uint8_t key_old;				//上一帧按键状态
extern uint8_t key_down;			//刚按下
extern uint8_t key_up;				//刚松开

#endif
