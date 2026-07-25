// C++ INCLUDES
#include <stdio.h>

// ESP32
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "driver/ledc.h"
#include "hal/ledc_types.h"
#include "freertos/FreeRTOS.h"
#include "esp_bt.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

#define LED_ERROR 1
#define LED_OK 0
#define LED_HIT 2

#define GAP_TAG "BLE_INIT"
#define DEVICE_NAME "GlaveSlave"

void led_indicate_status(int status){
    switch (status)
    {
    case LED_ERROR:
        while(1){
            gpio_set_level(GPIO_NUM_2, 1);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_NUM_2, 0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        break;
    case LED_HIT:
        gpio_set_level(GPIO_NUM_2, 1);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_NUM_2, 0);
        break;
    default:
        gpio_set_level(GPIO_NUM_2, 1);
    }

}

void pin_init(){
    gpio_config_t gpio_LED_PIN = {
        .pin_bit_mask = 1 << GPIO_NUM_2,
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&gpio_LED_PIN);
}

static void gatts_server_handler(
    esp_gatts_cb_event_t event,
    esp_gatt_if_t gatts_if,
    esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
        case ESP_GATTS_REG_EVT:
            break;

        case ESP_GATTS_CONNECT_EVT:
            break;

        case ESP_GATTS_DISCONNECT_EVT:
            break;

        default:
            break;
    }
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(GAP_TAG, "Advertising data set complete");
            esp_ble_adv_params_t adv_params = {
                .adv_int_min = 0x20,
                .adv_int_max = 0x40,
                .adv_type = ADV_TYPE_IND,
                .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
                .channel_map = ADV_CHNL_ALL,
                .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
            };
            esp_ble_gap_start_advertising(&adv_params);
            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(GAP_TAG, "Advertising started successfully!");
                led_indicate_status(LED_OK);
            } else {
                ESP_LOGE(GAP_TAG, "Advertising start failed");
                led_indicate_status(LED_ERROR);
            }
            break;

        default:
            break;
    }
}

esp_err_t bluetooth_setup() {
    esp_err_t ret;

    // 1. NVS init (с проверкой!)
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
        ESP_LOGE(GAP_TAG, "Controller init failed: %s", esp_err_to_name(ret));
        led_indicate_status(LED_ERROR);
    }
    led_indicate_status(LED_HIT);

    // 3. Enable controller
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(GAP_TAG, "Controller enable failed: %s", esp_err_to_name(ret));
        led_indicate_status(LED_ERROR);
    }
    ESP_LOGI(GAP_TAG, "Bluetooth controller initialized!");
    led_indicate_status(LED_HIT);

    // 4. Bluedroid
    esp_bluedroid_config_t cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ret = esp_bluedroid_init_with_cfg(&cfg);
    if(ret != ESP_OK){
        ESP_LOGE(GAP_TAG, "Bluedroid init failed: %s", esp_err_to_name(ret));
        led_indicate_status(LED_ERROR);
    }
    ESP_LOGI(GAP_TAG, "Bluedroid initialized!");
    led_indicate_status(LED_HIT);

    ret = esp_bluedroid_enable();
    if(ret != ESP_OK){
        ESP_LOGE(GAP_TAG, "Bluedroid enable failed: %s", esp_err_to_name(ret));
        led_indicate_status(LED_ERROR);
    }
    ESP_LOGI(GAP_TAG, "Bluedroid enabled!");
    led_indicate_status(LED_HIT);

    // 5.Reg GAP callback
    esp_ble_gap_register_callback(gap_event_handler);

    // 6. Reg GATTS
    esp_ble_gatts_register_callback(gatts_server_handler);

    // 7. Set device name
    esp_ble_gap_set_device_name(DEVICE_NAME);
    
    // 8. Seting ADV Data
        esp_ble_adv_data_t adv_data = {
        .include_name = true,
        .flag = ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT,
    };
    esp_ble_gap_config_adv_data(&adv_data);

    ESP_LOGI(GAP_TAG, "Advertising data configured, waiting for callback...");

    led_indicate_status(LED_OK);
    return ESP_OK;
}

void app_main(){
    pin_init();
    bluetooth_setup();

}