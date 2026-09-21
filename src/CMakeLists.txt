idf_component_register(

    SRCS

        main.c
        app/app.c
        ble/ble.c
        ble/modules/gap.c
        ble/modules/hid.c
        drivers/board_led.c
        drivers/buttons.c
        drivers/oled.c
        drivers/adc.c
        board/board.c

    INCLUDE_DIRS

        "."
        "app"
        "ble"
        "ble/modules"
        "drivers"
        "board"

    PRIV_REQUIRES
        bt

    REQUIRES
        esp_driver_i2c
        u8g2
)