//C++ INCLUDES

// HEADERS
#include "gap.h"

// ESP32
#include "esp_gap_ble_api.h"
#include "freertos/FreeRTOS.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_log.h"
#include "esp_bt_device.h"

#define GAP_TAG "GAP"

static esp_ble_adv_data_t adv_data = {
    .include_name = true,
    .flag = ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT,
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(GAP_TAG, "Adv data set, starting advertising");
            esp_ble_gap_start_advertising(&adv_params);
            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(GAP_TAG, "Advertising started successfully!");
            } else {
                ESP_LOGE(GAP_TAG, "Advertising start failed");
            }
            break;

        default:
            break;
    }
}

esp_err_t gap_register(void) {
    return esp_ble_gap_register_callback(gap_event_handler);
}

esp_err_t gap_start(void) {
    esp_err_t ret = esp_ble_gap_set_device_name("GlaveSlave");
    if (ret != ESP_OK) {
        ESP_LOGE(GAP_TAG, "Set device name failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_ble_gap_config_adv_data(&adv_data);
    if (ret != ESP_OK) {
        ESP_LOGE(GAP_TAG, "Config adv data failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(GAP_TAG, "GAP start initiated, waiting for callback...");
    return ESP_OK;
}

void gap_stop(void) {
    esp_ble_gap_stop_advertising();
    ESP_LOGI(GAP_TAG, "Advertising stopped");
}