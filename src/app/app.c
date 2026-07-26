//C++ INCLUDES

// HEADERS
#include "app.h"
#include "../ble/ble.h"
#include "../ble/modules/hid.h"
// ESP32
#include "freertos/FreeRTOS.h"

void app_init(){
    ble_init();
    while (1) {
        hid_send_mouse_report(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}