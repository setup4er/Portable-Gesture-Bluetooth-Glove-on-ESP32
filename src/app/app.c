#include "app.h"

#include "../ble/ble.h"
#include "../ble/modules/hid.h"

#include "../drivers/buttons.h"
#include "../drivers/oled.h"
#include "../drivers/adc.h"
#include "../drivers/bmi160.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdbool.h>
#include <stdint.h>

bool button_state[BTN_COUNT];

static int dx = 0;
static int dy = 0;


/*
 * Mouse loop.
 *
 * 100 Hz keeps input smooth and avoids a zero-tick
 * xTaskDelayUntil() on a 100 Hz FreeRTOS tick.
 */
static void mouse_task(void *arg)
{
    TickType_t last_wake = xTaskGetTickCount();

    while (1)
    {
        buttons_click_event();
        buttons_get_states(button_state);

        if (buttons_is_hold_active())
        {
            dx = 0;
            dy = 0;
        }
        else
        {
            bmi160_get_mouse_delta(&dx, &dy);
        }

        /*
         * IMPORTANT:
         * The mouse movement report must contain the current
         * LMB/RMB state.
         *
         * Sending 0 here would release the button on every
         * movement report and make drag-and-drop impossible.
         */
        uint8_t button_mask = 0;

        if (button_state[BTN_LMB])
            button_mask |= 0x01;

        if (button_state[BTN_RMB])
            button_mask |= 0x02;

        if (dx != 0 || dy != 0)
        {
            hid_send_mouse_report(
                dx,
                dy,
                button_mask
            );
        }

        vTaskDelayUntil(
            &last_wake,
            pdMS_TO_TICKS(10)
        );
    }
}


/*
 * OLED is intentionally updated much slower than the mouse.
 * This prevents OLED I2C traffic from disturbing cursor motion.
 */
static void oled_task(void *arg)
{
    TickType_t last_wake = xTaskGetTickCount();
    TickType_t last_battery = xTaskGetTickCount();

    float filtered_voltage = 0.0f;
    bool battery_initialized = false;

    while (1)
    {
        oled_update_ui(
            host_is_connected(),
            button_state
        );

        if (
            xTaskGetTickCount() - last_battery >=
            pdMS_TO_TICKS(1000)
        )
        {
            last_battery = xTaskGetTickCount();

            float voltage = adc_get_voltage();

            if (voltage > 0.0f)
            {
                if (!battery_initialized)
                {
                    filtered_voltage = voltage;
                    battery_initialized = true;
                }
                else
                {
                    filtered_voltage =
                        filtered_voltage * 0.90f +
                        voltage * 0.10f;
                }

                oled_set_battery_percent(
                    adc_voltage_to_percent(filtered_voltage)
                );
            }
        }

        vTaskDelayUntil(
            &last_wake,
            pdMS_TO_TICKS(100)
        );
    }
}


void app_init(void)
{
    ble_init();

    oled_print_welcome_screen();

    vTaskDelay(pdMS_TO_TICKS(1000));

    xTaskCreate(
        mouse_task,
        "mouse_task",
        4096,
        NULL,
        10,
        NULL
    );

    xTaskCreate(
        oled_task,
        "oled_task",
        4096,
        NULL,
        3,
        NULL
    );

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}