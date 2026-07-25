// HEADERS
#include "hid.h"
#include "gap.h"

// ESP32
#include "esp_log.h"
#include "esp_hidd_api.h"
#include "esp_err.h"

#define HID_TAG "HID"

static bool hid_ready = false;

static void hidd_event_handler(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param) {
    switch (event) {
        case ESP_HIDD_INIT_EVT:
            ESP_LOGI(HID_TAG, "HID init done, registering app...");
            
            esp_hidd_app_param_t app_param = {
                .name = "Gesture Glove 0.1",
                .description = "Портативная Bluetooth перчатка-мышь с гироскопом.",
                .subclass = ESP_HID_CLASS_MIC
            };
            esp_hidd_qos_param_t in_qos = {0};
            esp_hidd_qos_param_t out_qos = {0};
            
            esp_err_t ret = esp_bt_hid_device_register_app(&app_param, &in_qos, &out_qos);
            if (ret != ESP_OK) {
                ESP_LOGE(HID_TAG, "Register app failed: %s", esp_err_to_name(ret));
            }
            break;

        case ESP_HIDD_REGISTER_APP_EVT:
            ESP_LOGI(HID_TAG, "HID app registered successfully!");
            hid_ready = true;
            gap_start();
            break;

        default:
            break;
    }
}

esp_err_t hid_init(void) {
    esp_err_t ret;

    ret = esp_bt_hid_device_register_callback(hidd_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(HID_TAG, "Register callback failed: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ret = esp_bt_hid_device_init();
    if (ret != ESP_OK) {
        ESP_LOGE(HID_TAG, "Init failed: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }
    ESP_LOGI(HID_TAG, "HID init started, waiting for callback...");
    return ESP_OK;
}