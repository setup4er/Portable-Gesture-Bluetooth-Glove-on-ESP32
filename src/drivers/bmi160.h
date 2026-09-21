#pragma once

#include <stdbool.h>

void bmi160_init(void);
void bmi160_calibrate(void);
void bmi160_set_zero_position(void);
bool bmi160_is_ready(void);
bool bmi160_get_mouse_delta(int *dx, int *dy);
