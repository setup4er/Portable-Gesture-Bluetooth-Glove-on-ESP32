//C++ INCLUDES

// HEADERS
#include "app.h"
#include "../ble/ble.h"
#include "../ble/modules/hid.h"
#include "../drivers/buttons.h"
#include "../drivers/oled.h"
// ESP32
#include "freertos/FreeRTOS.h"

void app_init(){
    ble_init();
    while (1) {
        oled_update_ui(host_is_connected());
        int dx, dy;
        // calculate_delta_from_bno055(&dx, &dy); // Вычисления изменения векторов контроллера bno055
        buttons_click_event();
        
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