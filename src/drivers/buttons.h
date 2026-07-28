#ifndef BUTTONS_H
#define BUTTONS_H

/**
 * @file buttons.h
 * @date 2026-07-28
 * @description Buttons init modul
 */

#define RMB_BUTTON GPIO_NUM_26
#define LMB_BUTTON GPIO_NUM_25

void buttons_click_event();
void buttons_init();

#endif // BUTTONS_H