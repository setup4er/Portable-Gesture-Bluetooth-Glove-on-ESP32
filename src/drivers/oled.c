#include "oled.h"
#include "board_led.h"
#include "adc.h"

#include "esp_log.h"
#include "u8g2.h"
#include "u8g2_port_esp32_hw_i2c.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

#define OLED_TAG "OLED"

#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH  128
#define OLED_HEIGHT 64


static u8g2_t u8g2;

bool bluetooth_icon_visible = false; // Icon in NO DEVICE screen

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

// WARNING! Startup print function has SendBuffer.
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
        u8g2_font_fur11_tf
    );

    u8g2_DrawStr(
        &u8g2,
        10,
        35,
        "WELCOME"
    );
    u8g2_SendBuffer(&u8g2);
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
    if(!bluetooth_icon_visible){
        oled_print_bluetooth_connected_icon(56, 56);
        bluetooth_icon_visible = true;
    }else{
        bluetooth_icon_visible = false;
    }
}

void oled_print_pressed_button(bool button_state[BTN_COUNT]){

    u8g2_SetFont(&u8g2, u8g2_font_6x12_tf);

    for(int i = 0; i < BTN_COUNT; i++){
        if(i == ID_LMB_BUTTON && button_state[i]){
            u8g2_DrawStr(&u8g2, 22, 12, "LMB");
        }
        if(i == ID_HOLD_BUTTON && button_state[i]){
            u8g2_DrawStr(&u8g2, 44, 12, "HOLD");
        }
        if(i == ID_RMB_BUTTON && button_state[i]){
            u8g2_DrawStr(&u8g2, 72, 12, "RMB");
        }
    }
}

void oled_print_table_items(){
    u8g2_DrawLine(&u8g2, 0, 17, 127, 17);   // горизонтальная

    u8g2_DrawLine(&u8g2, 42, 0, 42, 17);
    u8g2_DrawLine(&u8g2, 70, 0, 70, 17);
}

void oled_print_battery_percent(void)
{
    int percent = adc_get_percent();

    // Иконка батарейки: корпус 16x9 px в правом верхнем углу
    const u8g2_uint_t bx = 110;   // x корпуса
    const u8g2_uint_t by = 1;     // y корпуса
    const u8g2_uint_t bw = 16;    // ширина корпуса
    const u8g2_uint_t bh = 9;     // высота корпуса

    // Корпус
    u8g2_DrawFrame(&u8g2, bx, by, bw, bh);

    // "Пимпочка" справа
    u8g2_DrawBox(&u8g2, bx + bw, by + 3, 2, 3);

    // Заполнение по проценту (внутренняя область 14x7)
    u8g2_uint_t fill_w = (u8g2_uint_t)((bw - 2) * percent / 100);
    if (fill_w > 0) {
        u8g2_DrawBox(&u8g2, bx + 1, by + 1, fill_w, bh - 2);
    }

    // Текст процента слева от иконки
    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", percent);

    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    u8g2_uint_t text_w = u8g2_GetStrWidth(&u8g2, buf);
    u8g2_DrawStr(&u8g2, bx - text_w - 3, by + 7, buf);
}

// MAIN FUNCTION IN EXT. CYCLE
void oled_update_ui(bool host_is_connected, bool button_state[BTN_COUNT])
{
    u8g2_ClearBuffer(&u8g2);
    
    if (!host_is_connected) {
        oled_print_bluetooth_disconnect_screen();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }else{
        oled_print_table_items();
        oled_print_bluetooth_connected_icon(0, 16);
        oled_print_pressed_button(button_state);
    }

    oled_print_battery_percent();   
    
    u8g2_SendBuffer(&u8g2);
}