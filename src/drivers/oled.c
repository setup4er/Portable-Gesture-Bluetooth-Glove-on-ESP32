#include "oled.h"
#include "board_led.h"

#include "esp_log.h"
#include "u8g2.h"
#include "u8g2_port_esp32_hw_i2c.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

#define OLED_TAG "OLED"

#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH  128
#define OLED_HEIGHT 64

#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22

static u8g2_t u8g2;

bool is_bluetooth_icon_display = false; // Icon in NO DEVICE screen

// CONFIG I2C
const u8g2_esp32_i2c_config_t i2c_cfg = {
    .i2c_port = I2C_NUM_0,
    .sda_pin = SDA_PIN,
    .scl_pin = SCL_PIN,
    .dev_addr_7bit = OLED_I2C_ADDRESS,
    .clk_hz = 400000,
    .timeout_ms = 1000,
};

u8g2_esp32_i2c_ctx_t i2c_ctx = {
    .cfg = i2c_cfg,
    .bus_handle = NULL,
    .dev_handle = NULL,
    .tx_buf = {0},
    .tx_len = 0,
    .current_addr_7bit = OLED_I2C_ADDRESS,
    .initialized = 0,
};

void oled_init(void)
{
    ESP_LOGI(OLED_TAG, "Initializing U8g2 OLED...");

    u8g2_esp32_i2c_set_default_context(&i2c_ctx);

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
        &u8g2,
        U8G2_R0,
        u8x8_byte_esp32_hw_i2c,
        u8x8_gpio_and_delay_esp32_i2c
    );

    u8x8_SetI2CAddress(
        &u8g2.u8x8,
        OLED_I2C_ADDRESS << 1
    );

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    ESP_LOGI(OLED_TAG, "OLED initialized");
    led_indicate_status(LED_HIT);
}

void oled_print_welcome_screen(void){
    u8g2_ClearBuffer(&u8g2);

    u8g2_DrawFrame(
        &u8g2,
        0,
        0,
        OLED_WIDTH,
        OLED_HEIGHT
    );

    u8g2_SetFont(
        &u8g2,
        u8g2_font_6x10_tf
    );

    u8g2_DrawStr(
        &u8g2,
        10,
        35,
        "WELCOME"
    );

    u8g2_SendBuffer(&u8g2);

    vTaskDelay(3000 / portTICK_PERIOD_MS);
}

void oled_print_bluetooth_connected_icon(u8g2_uint_t x0, u8g2_uint_t y0)
{
    u8g2_SetFont(&u8g2, u8g2_font_open_iconic_embedded_2x_t);
    u8g2_DrawGlyph(&u8g2, x0, y0, 0x004A);
}

void oled_print_bluetooth_disconnect_screen(){
    u8g2_SetFont(&u8g2, u8g2_font_fur17_tf);
    u8g2_DrawStr(
        &u8g2,
        0, 33,
        "NO DEVICE"
    );

    if(!is_bluetooth_icon_display){
        oled_print_bluetooth_connected_icon(56, 56);
        is_bluetooth_icon_display = true;
    }else{
        is_bluetooth_icon_display = false;
    }
    u8g2_SendBuffer(&u8g2);
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

void oled_update_ui(bool host_is_connected)
{
    u8g2_ClearBuffer(&u8g2);

    if (!host_is_connected) {
        oled_print_bluetooth_disconnect_screen();
        return;
    }
    oled_print_bluetooth_connected_icon(0, 16);

    u8g2_SendBuffer(&u8g2);
}