//C++ INCLUDES

// HEADERS
#include "board.h"
#include "../drivers/board_led.h"
#include "../drivers/buttons.h"

// ESP32

void board_init(){
    led_init();
    buttons_init();
}