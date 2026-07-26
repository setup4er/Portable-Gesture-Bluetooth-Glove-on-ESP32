// HEADERS
#include "hid.h"
#include "gap.h"

// ESP32
#include "esp_log.h"
#include "esp_hidd_api.h"
#include "esp_err.h"

#define HID_TAG "HID"

static bool hid_ready = false;

static const uint8_t hid_mouse_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Buttons)
    0x19, 0x01,        //     Usage Minimum (1)
    0x29, 0x03,        //     Usage Maximum (3)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data, Variable, Absolute)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5) — padding
    0x81, 0x03,        //     Input (Constant)
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x06,        //     Input (Data, Variable, Relative)
    0xC0,              //   End Collection
    0xC0               // End Collection
};

void hid_send_mouse_report(int8_t x, int8_t y, uint8_t buttons) {
    uint8_t report[4] = {buttons, x, y, 0};
    esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INPUT, 0, 4, report);
}

static void hidd_event_handler(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param) {
    switch (event) {
        case ESP_HIDD_INIT_EVT:
            ESP_LOGI(HID_TAG, "HID init done, registering app...");
            
            static esp_hidd_app_param_t app_param = {
                .name = "Gesture Glove 0.1",
                .description = "BT mouse with gyroscope",
                .desc_list = (uint8_t*)hid_mouse_descriptor,
                .desc_list_len = sizeof(hid_mouse_descriptor),
                .provider = "Espressif",
                .subclass = ESP_HID_CLASS_MIC,
            };
            static esp_hidd_qos_param_t in_qos = {0};
            static esp_hidd_qos_param_t out_qos = {0};
            
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
        case ESP_HIDD_OPEN_EVT:
            ESP_LOGI(HID_TAG, "Host connected (OPEN)");
            hid_send_mouse_report(0, 0, 0);
            break;
        case ESP_HIDD_CLOSE_EVT:
            ESP_LOGI(HID_TAG, "Host disconnected (CLOSE)");
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