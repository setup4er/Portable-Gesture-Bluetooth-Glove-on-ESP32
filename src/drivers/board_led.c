//C++ INCLUDES

// HEADERS
#include "board_led.h"

// ESP32
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

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

void led_init(){
    gpio_config_t gpio_LED_PIN = {
        .pin_bit_mask = 1 << GPIO_NUM_2,
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&gpio_LED_PIN);
}