//C++ INCLUDES

// HEADERS
#include "app.h"
#include "../ble/ble.h"
#include "../ble/modules/hid.h"
#include "../drivers/buttons.h"
#include "../drivers/oled.h"
// ESP32
#include "freertos/FreeRTOS.h"

bool button_state[BTN_COUNT]; // Array from buttons getter

int dx, dy; // Shift cursor from BNO


void app_init(){
    ble_init();
    oled_print_welcome_screen();
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    while (1) {
        oled_update_ui(host_is_connected(), button_state);

        // calculate_delta_from_gyroscope(&dx, &dy); // Вычисления изменения векторов контроллера bno055
        buttons_click_event();
        buttons_get_states(button_state);

        if (buttons_is_hold_active()) {
            dx = 0;
            dy = 0;
        }
        if(dx != 0 || dy != 0){
            hid_send_mouse_report(dx, dy, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(40));
    }
}