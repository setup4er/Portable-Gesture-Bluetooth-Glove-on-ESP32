#ifndef HID_H
#define HID_H

/**
 * @file hid.h
 * @date 2026-07-25
 * @description Setup HID Service
 */

#include "esp_err.h"

esp_err_t hid_init(void);
void hid_send_mouse_report(int8_t x, int8_t y, uint8_t buttons);
bool host_is_connected();

#endif // HID_H