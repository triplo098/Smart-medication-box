#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <nvs_flash.h>

#include "display_driver.h"

#define CONFIG_EXAMPLE_I2C_MASTER_SDA 5
#define CONFIG_EXAMPLE_I2C_MASTER_SCL 6

static const char *TAG = "main";
i2c_dev_t rtc_dev;
i2c_dev_t chsc6x_dev;

lv_obj_t *time_label;
lv_indev_t *indev;

void app_main(void)
{

    // I2c iniat
    memset(&rtc_dev, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(i2cdev_init());

    // PCF8563 RTC
    ESP_ERROR_CHECK(pcf8563_init_desc(&rtc_dev, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));

    struct tm time = {0};
    strptime(__DATE__, "%b %d %Y", &time);
    strptime(__TIME__, "%H:%M:%S", &time);

    ESP_ERROR_CHECK(pcf8563_set_time(&rtc_dev, &time));

    // Display driver and LVGL init
    display_init();

    lv_init();
    lv_tick_set_cb(xTaskGetTickCount);

    lv_display_t *display = lv_display_create(EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);
    static uint8_t buf[EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES / 20]; // Buffer size set for 240x240 display

    lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, my_flush_cb); // Set the flush callback

    // Setting up touch I2c
    memset(&chsc6x_dev, 0, sizeof(i2c_dev_t));
    chsc6x_init_desc(&chsc6x_dev, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL);
    touch_init();

    // Create a label to display time
    time_label = lv_label_create(lv_scr_act());
    lv_obj_center(time_label);
    lv_timer_create(lvgl_update_time_cb, 100, NULL);

    ESP_LOGI(TAG, "Entering main loop");
    while (1)
    {
        // Handle LVGL tasks
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}