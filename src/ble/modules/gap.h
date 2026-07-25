#ifndef GAP_H
#define GAP_H

/**
 * @file gap.h
 * @date 2026-07-25
 * @description Инициализация и настройка GAP
 */

#include "esp_err.h"

esp_err_t gap_register(void);
esp_err_t gap_start(void);
void gap_stop(void);

#endif // GAP_H