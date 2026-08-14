#ifndef OLED_H
#define OLED_H

/**
 * @file oled.h
 * @date 2026-08-12
 * @description Oled init file
 */

void oled_init();
void oled_clear();
void oled_print_bluetooth_connected_icon();
void oled_update_ui(bool host_is_connected);

#endif // OLED_H