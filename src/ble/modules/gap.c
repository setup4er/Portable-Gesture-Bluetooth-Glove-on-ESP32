// HEADERS
#include "gap.h"

// ESP32
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define GAP_TAG "GAP"
#define DEVICE_NAME "GlaveSlave"

static void gap_event_handler(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch (event) {
        case ESP_BT_GAP_AUTH_CMPL_EVT:
            ESP_LOGI(GAP_TAG, "Authentication complete");
            break;
        default:
            break;
    }
}

esp_err_t gap_register(void) {
    return esp_bt_gap_register_callback(gap_event_handler);  // ← esp_bt_*, не esp_ble_*
}

esp_err_t gap_start(void) {
    // Устанавливаем имя устройства
    esp_bt_gap_set_device_name(DEVICE_NAME);
    
    // Делаем устройство видимым и подключаемым
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    
    ESP_LOGI(GAP_TAG, "Device is discoverable as '%s'", DEVICE_NAME);
    return ESP_OK;
}

void gap_stop(void) {
    esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
    ESP_LOGI(GAP_TAG, "Advertising stopped");
}