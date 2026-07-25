//C++ INCLUDES

// HEADERS
#include "board.h"
#include "../drivers/board_led.h"

// ESP32

void board_init(){
    led_init();
}