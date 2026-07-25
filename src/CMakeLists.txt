idf_component_register(
    SRCS 
        main.c                      
        app/app.c
        ble/ble.c
        ble/modules/gap.c
        drivers/board_led.c
        board/board.c
    INCLUDE_DIRS 
        "."
        "app"
        "ble"
        "ble/modules"
        "drivers"
        "board"
    PRIV_REQUIRES bt
)