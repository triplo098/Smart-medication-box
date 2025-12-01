idf_component_register(
    SRCS 
        "medicines_managment_test.c"
        # Include source files you want to test
        "../../main/alarm_helpers.c"
        "../../main/medicines_managment.c"
        "../../main/alarm_helpers.c"
    INCLUDE_DIRS 
        "."
        "../../main/include"
        # "../../alarm_helpers.c"
    REQUIRES 
        unity
        driver
        pcf8563
        lvgl__lvgl
        # nvs_flash
)