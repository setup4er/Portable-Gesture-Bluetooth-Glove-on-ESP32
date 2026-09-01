#ifndef BOARD_LED_H
#define BOARD_LED_H

/**
 * @file board_led.h
 * @date 2026-07-25
 * @description Светодиод на плате
 */
#define LED_ERROR 1
#define LED_HIT 2
#define LED_OFF 0

void led_indicate_status(int status);
void led_init();

#endif // BOARD_LED_H