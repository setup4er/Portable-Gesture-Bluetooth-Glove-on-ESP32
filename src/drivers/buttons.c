//C++ INCLUDES

// HEADERS
#include "buttons.h"
#include "hid.h"

// ESP32
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define BTN_TAG "BTN"
#define DEBOUNCE_MS 30

#define MASK_LMB 0x01
#define MASK_RMB 0x02

static uint8_t prev_mask = 0;
static int64_t last_lmb_change = 0;
static int64_t last_rmb_change = 0;
static int64_t last_hold_change = 0;

static volatile bool hold_active = false;

void buttons_init(){
    gpio_reset_pin(LMB_BUTTON);
    gpio_reset_pin(RMB_BUTTON);
    gpio_reset_pin(HOLD_BUTTON);

    gpio_config_t btn_config = {
        .pin_bit_mask =
            (1ULL << LMB_BUTTON) |
            (1ULL << RMB_BUTTON) |
            (1ULL << HOLD_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&btn_config);
}

void buttons_click_event(){
    int64_t now = esp_timer_get_time() / 1000; // мс

    // Кнопка замыкает на GND -> при нажатии LOW (0)
    bool lmb_raw  = (gpio_get_level(LMB_BUTTON)  == 1);
    bool rmb_raw  = (gpio_get_level(RMB_BUTTON)  == 1);
    bool hold_raw = (gpio_get_level(HOLD_BUTTON) == 1);

    uint8_t new_mask = prev_mask;

    // --- LMB ---
    bool lmb_current = (prev_mask & MASK_LMB) != 0;
    if (lmb_raw != lmb_current && (now - last_lmb_change) > DEBOUNCE_MS) {
        last_lmb_change = now;
        if (lmb_raw) new_mask |= MASK_LMB;
        else         new_mask &= ~MASK_LMB;
        ESP_LOGI(BTN_TAG, "LMB %s", lmb_raw ? "pressed" : "released");
    }

    // --- RMB ---
    bool rmb_current = (prev_mask & MASK_RMB) != 0;
    if (rmb_raw != rmb_current && (now - last_rmb_change) > DEBOUNCE_MS) {
        last_rmb_change = now;
        if (rmb_raw) new_mask |= MASK_RMB;
        else         new_mask &= ~MASK_RMB;
        ESP_LOGI(BTN_TAG, "RMB %s", rmb_raw ? "pressed" : "released");
    }
    
    // --- HOLD (внутренний флаг) ---
    if (hold_raw != hold_active && (now - last_hold_change) > DEBOUNCE_MS) {
        last_hold_change = now;
        hold_active = hold_raw;
        ESP_LOGI(BTN_TAG, "HOLD %s", hold_active ? "engaged" : "released");
    }

    if (new_mask != prev_mask) {
        hid_send_mouse_report(0, 0, new_mask);
        prev_mask = new_mask;
    }
}

bool buttons_is_hold_active(void){
    return hold_active;
}