//C++ INCLUDES

// HEADERS
#include "board.h"
#include "../drivers/board_led.h"
#include "../drivers/buttons.h"
#include "../drivers/oled.h"
#include "../drivers/adc.h"
#include "../drivers/bmi160.h"

// ESP32

void board_init(){
    led_init();
    buttons_init();
    oled_init();
    adc_init();
    bmi160_init();
}