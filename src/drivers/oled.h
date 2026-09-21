#ifndef OLED_H
#define OLED_H

/**
 * @file oled.h
 * @date 2026-08-12
 * @description Oled init file
 */
#include "driver/i2c_master.h"


#define SDA_PIN     GPIO_NUM_21
#define SCL_PIN     GPIO_NUM_22

#define ID_LMB_BUTTON   0
#define ID_RMB_BUTTON   1
#define ID_HOLD_BUTTON  2

#define BTN_COUNT   3

i2c_master_bus_handle_t oled_get_i2c_bus(void);
void oled_init();
void oled_update_ui(bool host_is_connected, bool button_state[BTN_COUNT]);
void oled_print_welcome_screen(void);
void oled_set_battery_percent(int percent);

#endif // OLED_H