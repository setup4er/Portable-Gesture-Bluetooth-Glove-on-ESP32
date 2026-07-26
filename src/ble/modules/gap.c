#include "gap.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"

#define GAP_TAG "GAP"
#define DEVICE_NAME "GlaveSlave"

static void gap_event_handler(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch (event) {
        case ESP_BT_GAP_AUTH_CMPL_EVT:
            ESP_LOGI(GAP_TAG, "Authentication complete, status: %d", param->auth_cmpl.stat);
            break;
        default:
            break;
    }
}

esp_err_t gap_register(void) {
    return esp_bt_gap_register_callback(gap_event_handler);
}

esp_err_t gap_start(void) {
    esp_err_t ret = esp_bt_gap_set_device_name(DEVICE_NAME);
    if (ret != ESP_OK) {
        ESP_LOGE(GAP_TAG, "Set name failed");
        return ret;
    }

    esp_bt_cod_t cod = {
        .major = ESP_BT_COD_MAJOR_DEV_PERIPHERAL,
        .minor = ESP_BT_COD_MINOR_PERIPHERAL_POINTING,
        .service = ESP_BT_COD_SRVC_NONE,
        .reserved_2 = 0
    };
    ret = esp_bt_gap_set_cod(cod, ESP_BT_SET_COD_ALL);
    if (ret != ESP_OK) {
        ESP_LOGE(GAP_TAG, "Set COD failed");
        return ret;
    }

    ret = esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    if (ret != ESP_OK) {
        ESP_LOGE(GAP_TAG, "Set scan mode failed");
        return ret;
    }

    ESP_LOGI(GAP_TAG, "Device is discoverable as '%s'", DEVICE_NAME);
    return ESP_OK;
}

void gap_stop(void) {
    esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
}