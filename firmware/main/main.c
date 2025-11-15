#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_sleep.h>

#include "display_driver.h"
#include "lvgl_default_scr.h"
#include "motor_driver.h"
#include "medicines_managment.h"
#include "alarm_helpers.h"



i2c_dev_t rtc_dev;      // PCF8563 RTC device descriptor
i2c_dev_t chsc6x_dev;   // CHSC6X Touch controller device descriptor

lv_obj_t *time_label;   // LVGL label to display time
lv_indev_t *indev;      // LVGL input device for touch

static const char *TAG = "main";

void lvgl_task(void *arg)
{   

    vTaskDelay(pdMS_TO_TICKS(100));
    turn_alarm(true);
    vTaskDelay(pdMS_TO_TICKS(5000));
    turn_alarm(false);
    vTaskDelay(pdMS_TO_TICKS(5000));
    turn_alarm(true);


    while (1)
    {
        lv_timer_handler(); // Handle LVGL tasks
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void app_main(void)
{
    
    // I2C init
    ESP_ERROR_CHECK(i2cdev_init());

    // PCF8563 RTC
    memset(&rtc_dev, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(pcf8563_init_desc(&rtc_dev, 0, I2C_MASTER_SDA_PIN, I2C_MASTER_SCL_PIN));

    struct tm time = {0};
    bool valid_time = false;
    pcf8563_get_time(&rtc_dev, &time, &valid_time);

    if (valid_time)
    {
        ESP_LOGI(TAG, "Current time is valid");
    }
    else
    {
        ESP_LOGW(TAG, "Current time is NOT valid, setting to compile time");
        strptime(__DATE__, "%b %d %Y", &time);
        strptime(__TIME__, "%H:%M:%S", &time);
        ESP_ERROR_CHECK(pcf8563_set_time(&rtc_dev, &time));
    }

    // Display driver and LVGL init
    display_init();
    lv_init();
    lv_tick_set_cb(xTaskGetTickCount);

    lv_display_t *display = lv_display_create(LCD_H_RES, LCD_V_RES);
    static uint8_t buf[LCD_H_RES * LCD_V_RES / 20]; // Buffer size set for 240x240 display

    lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, my_flush_cb); // Set the flush callback

    // Setting up touch I2c
    memset(&chsc6x_dev, 0, sizeof(i2c_dev_t));
    chsc6x_init_desc(&chsc6x_dev, 0, I2C_MASTER_SDA_PIN, I2C_MASTER_SCL_PIN);
    touch_init();


    // Default screen
    init_lvgl_default_scr(NULL);

    // Initialize motor driver
    init_motor();

    // Init alarm
    alarm_gpio_init();


    add_medicine((medicine_t){
        .name = "Medicine A",
        .doses_per_day = 2,
        .dose_times = {
            {.tm_hour = 9, .tm_min = 0},
            {.tm_hour = 21, .tm_min = 0}
        },
        .special_requirements = "Take after meals",
        .treatment_days_left = 10
    });

    add_medicine((medicine_t){
        .name = "Medicine B",
        .doses_per_day = 3,
        .dose_times = {
            {.tm_hour = 8, .tm_min = 30},
            {.tm_hour = 14, .tm_min = 30},
            {.tm_hour = 20, .tm_min = 30}
        },
        .special_requirements = "Take with water",
        .treatment_days_left = 5
    });
    
    
    ESP_LOGI(TAG, "Entering LVGL task loop");
    xTaskCreate(lvgl_task, "LVGL", 8192, NULL, 2, NULL);

}