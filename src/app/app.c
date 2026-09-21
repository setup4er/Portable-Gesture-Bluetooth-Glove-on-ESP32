// C++ INCLUDES

// HEADERS
#include "app.h"
#include "../ble/ble.h"
#include "../ble/modules/hid.h"
#include "../drivers/buttons.h"
#include "../drivers/oled.h"
#include "../drivers/adc.h"

// ESP32
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

bool button_state[BTN_COUNT];

int dx, dy;

static float filtered_voltage = 0.0f;
static bool battery_initialized = false;

static void update_battery()
{
    float voltage = adc_get_voltage();

    if (voltage <= 0.0f)
        return;

    if (!battery_initialized) {
        filtered_voltage = voltage;
        battery_initialized = true;
    } else {
        filtered_voltage =
            filtered_voltage * 0.90f +
            voltage * 0.10f;
    }

    oled_set_battery_percent(
        adc_voltage_to_percent(filtered_voltage)
    );
}

void app_init(){
    ble_init();
    oled_print_welcome_screen();
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    TickType_t last_battery_update = xTaskGetTickCount();

    while (1) {
        oled_update_ui(host_is_connected(), button_state);

        buttons_click_event();
        buttons_get_states(button_state);

        if (buttons_is_hold_active()) {
            dx = 0;
            dy = 0;
        }

        if(dx != 0 || dy != 0){
            hid_send_mouse_report(dx, dy, 0);
        }

        if(xTaskGetTickCount() - last_battery_update >= pdMS_TO_TICKS(1000)){
            last_battery_update = xTaskGetTickCount();
            update_battery();
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}