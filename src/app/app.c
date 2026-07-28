//C++ INCLUDES

// HEADERS
#include "app.h"
#include "../ble/ble.h"
#include "../ble/modules/hid.h"
#include "../drivers/buttons.h"
// ESP32
#include "freertos/FreeRTOS.h"

void app_init(){
    ble_init();
    while (1) {
        buttons_click_event();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}