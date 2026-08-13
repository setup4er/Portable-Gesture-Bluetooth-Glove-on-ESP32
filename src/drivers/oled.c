#include "oled.h"
#include "board_led.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "esp_lcd_io_i2c.h"
#include "esp_lcd_panel_ssd1306.h"
#include "esp_lcd_panel_ops.h"

#define OLED_TAG "OLED"
#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22

static esp_lcd_panel_handle_t panel_handle = NULL;
static uint8_t blank_buffer[OLED_WIDTH * OLED_HEIGHT / 8] = {0};


void screen_clear(){
    if (panel_handle != NULL) {
        esp_err_t ret = esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, OLED_WIDTH, OLED_HEIGHT, blank_buffer);
        if (ret != ESP_OK) {
            ESP_LOGE(OLED_TAG, "Failed to clear screen: %s", esp_err_to_name(ret));
        }
        ESP_LOGI(OLED_TAG, "Screen cleared.");
    } else {
        ESP_LOGE(OLED_TAG, "Panel handle is NULL. Cannot clear screen.");
    }
}

void oled_init()
{
    esp_err_t ret;

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t i2c_bus = NULL;
    ret = i2c_new_master_bus(&bus_config, &i2c_bus);
    if (ret != ESP_OK) {
        ESP_LOGE(OLED_TAG, "I2C bus init failed");
        led_indicate_status(LED_ERROR);
        return;
    }
    ESP_LOGI(OLED_TAG, "I2C bus initialized successfully.");
    led_indicate_status(LED_HIT);

    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = OLED_I2C_ADDRESS,
        .scl_speed_hz = 400000,
        .control_phase_bytes = 1,
        .dc_bit_offset = 0,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };

    esp_lcd_panel_io_handle_t io_handle = NULL;
    ret = esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(OLED_TAG, "I2C IO init failed");
        led_indicate_status(LED_ERROR);
        return;
    }
    ESP_LOGI(OLED_TAG, "I2C IO initialized successfully.");
    led_indicate_status(LED_HIT);

    esp_lcd_panel_ssd1306_config_t ssd1306_cfg = {
        .height = OLED_HEIGHT,
    };

    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = -1,
        .vendor_config = &ssd1306_cfg,
    };

    ret = esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(OLED_TAG, "Panel init failed");
        led_indicate_status(LED_ERROR);
        return;
    }
    ESP_LOGI(OLED_TAG, "Panel initialized successfully.");
    led_indicate_status(LED_HIT);

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    ESP_LOGI(OLED_TAG, "OLED initialized");
    led_indicate_status(LED_HIT);
}