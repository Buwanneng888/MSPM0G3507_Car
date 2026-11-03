#include "bsp_system.h"
#include "button_driver.h"

uint8_t key_val, key_old, key_down, key_up;
static uint8_t key_last_pressed;

uint8_t key_read(void)
{
    uint8_t temp = 0U;

    if (DL_GPIO_readPins(KEY_KEY_MY_UP_PORT, KEY_KEY_MY_UP_PIN) == 0U) {
        temp |= KEY_MASK_MY_UP;
    }
    if (DL_GPIO_readPins(KEY_KEY_MY_DOWN_PORT, KEY_KEY_MY_DOWN_PIN) == 0U) {
        temp |= KEY_MASK_MY_DOWN;
    }
    if (DL_GPIO_readPins(KEY_KEY_LEFT_PORT, KEY_KEY_LEFT_PIN) == 0U) {
        temp |= KEY_MASK_LEFT;
    }
    if (DL_GPIO_readPins(KEY_KEY_RIGHT_PORT, KEY_KEY_RIGHT_PIN) == 0U) {
        temp |= KEY_MASK_RIGHT;
    }
    if (DL_GPIO_readPins(KEY_KEY_CENTER_PORT, KEY_KEY_CENTER_PIN) == 0U) {
        temp |= KEY_MASK_CENTER;
    }

    return temp;
}

static const char *key_name_from_mask(uint8_t key_mask)
{
    if ((key_mask & KEY_MASK_MY_UP) != 0U) {
        return "UP    ";
    }
    if ((key_mask & KEY_MASK_MY_DOWN) != 0U) {
        return "DOWN ";
    }
    if ((key_mask & KEY_MASK_LEFT) != 0U) {
        return "LEFT  ";
    }
    if ((key_mask & KEY_MASK_RIGHT) != 0U) {
        return "RIGHT  ";
    }
    if ((key_mask & KEY_MASK_CENTER) != 0U) {
        return "CENTER";
    }

    return "NONE  ";
}

const char *key_get_active_name(void)
{
    return key_name_from_mask(key_val);
}

const char *key_get_last_pressed_name(void)
{
    return key_name_from_mask(key_last_pressed);
}

__WEAK void key_task(void)
{
    key_val = key_read();
    key_down = key_val & (uint8_t)(key_old ^ key_val);
    key_up = (uint8_t)(~key_val) & (uint8_t)(key_old ^ key_val);
    key_old = key_val;

    if (key_down != 0U) {
        key_last_pressed = key_down;
    }
}
