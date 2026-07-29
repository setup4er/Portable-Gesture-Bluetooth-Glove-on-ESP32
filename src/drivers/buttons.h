#ifndef BUTTONS_H
#define BUTTONS_H

/**
 * @file buttons.h
 * @date 2026-07-28
 * @description Buttons init module
 */

#define RMB_BUTTON GPIO_NUM_25
#define LMB_BUTTON GPIO_NUM_26
#define HOLD_BUTTON GPIO_NUM_27

void buttons_click_event(); 
void buttons_init();

bool buttons_is_hold_active(void);

#endif // BUTTONS_H