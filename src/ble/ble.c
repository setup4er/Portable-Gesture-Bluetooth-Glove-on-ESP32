// HEADERS
#include "ble.h"
#include "../drivers/board_led.h"
#include "modules/gap.h"
#include "hid.h"

// ESP32
#include "esp_err.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define STACK_TAG "BLE STACK INIT"

void ble_stack_init(){
    esp_err_t ret;

    // 1. NVS init
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
        if(ret != ESP_OK){
            led_indicate_status(LED_ERROR);
        }
    }
    ESP_ERROR_CHECK(ret);
    led_indicate_status(LED_HIT);

    // 2. Controller init
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(STACK_TAG, "Controller init failed: %s", esp_err_to_name(ret));
        led_indicate_status(LED_ERROR);
    }
    led_indicate_status(LED_HIT);

    // 3. Enable controller
    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        ESP_LOGE(STACK_TAG, "Controller enable failed: %s", esp_err_to_name(ret));
        led_indicate_status(LED_ERROR);
    }
    ESP_LOGI(STACK_TAG, "Classic Bluetooth controller initialized!");
    led_indicate_status(LED_HIT);

    // 4. Bluedroid
    esp_bluedroid_config_t cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ret = esp_bluedroid_init_with_cfg(&cfg);
    if(ret != ESP_OK){
        ESP_LOGE(STACK_TAG, "Bluedroid init failed: %s", esp_err_to_name(ret));
        led_indicate_status(LED_ERROR);
    }
    ESP_LOGI(STACK_TAG, "Bluedroid initialized!");
    led_indicate_status(LED_HIT);

    ret = esp_bluedroid_enable();
    if(ret != ESP_OK){
        ESP_LOGE(STACK_TAG, "Bluedroid enable failed: %s", esp_err_to_name(ret));
        led_indicate_status(LED_ERROR);
    }
    ESP_LOGI(STACK_TAG, "Bluedroid enabled!");
    led_indicate_status(LED_HIT);
}

void ble_init(){
    ble_stack_init();
    gap_register();
    hid_init();
    led_indicate_status(LED_OK);
}