//C++ INCLUDES

// HEADERS
#include "buttons.h"
#include "hid.h"

// ESP32

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BTN_TAG "BTN"

void buttons_click_event(){
    if(gpio_get_level(LMB_BUTTON)){
        hid_send_mouse_report(0, 0, 2);
        ESP_LOGI(BTN_TAG, "LMB has been clicked successfully.");
    }
    if(gpio_get_level(RMB_BUTTON)){
        hid_send_mouse_report(0, 0, 1);
        ESP_LOGI(BTN_TAG, "RMB has been clicked successfully.");
    }
}

void buttons_init(){
    gpio_config_t btnObj_RMB = {
        .pin_bit_mask = 1 << RMB_BUTTON,
        .mode = GPIO_MODE_INPUT
    };
    gpio_config(&btnObj_RMB);

    gpio_config_t btnObj_LMB = {
        .pin_bit_mask = 1 << LMB_BUTTON,
        .mode = GPIO_MODE_INPUT
    };
    gpio_config(&btnObj_LMB);
}