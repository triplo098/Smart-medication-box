#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_sleep.h>
#include "esp_heap_caps.h"

#include "display_driver.h"
#include "lvgl_default_scr.h"
#include "motor_driver.h"
#include "medicines_managment.h"
#include "alarm_helpers.h"
#include "esp_task_wdt.h"





i2c_dev_t rtc_dev;      // PCF8563 RTC device descriptor
i2c_dev_t chsc6x_dev;   // CHSC6X Touch controller device descriptor

lv_obj_t *time_label;   // LVGL label to display time
lv_indev_t *indev;      // LVGL input device for touch

extern medicine_t** medicines_list;

struct tm current_time;

static const char *TAG = "main";

void lvgl_task(void *arg)
{   

    esp_task_wdt_add(NULL);

    while (1)
    {
        lv_timer_handler(); // Handle LVGL tasks
        vTaskDelay(pdMS_TO_TICKS(30));
        esp_task_wdt_reset();
    }
}


void app_main(void)
{
    
    // I2C init
    ESP_ERROR_CHECK(i2cdev_init());

    // PCF8563 RTC
    memset(&rtc_dev, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(pcf8563_init_desc(&rtc_dev, 0, I2C_MASTER_SDA_PIN, I2C_MASTER_SCL_PIN));


    bool valid_time = false;
    pcf8563_get_time(&rtc_dev, &current_time, &valid_time);

    
    valid_time = false;

    if (valid_time)
    {
        ESP_LOGI(TAG, "Current time is valid");
    }
    else
    {
        ESP_LOGW(TAG, "Current time is NOT valid, setting to compile time");
        strptime(__DATE__, "%b %d %Y", &current_time);
        strptime(__TIME__, "%H:%M:%S", &current_time);
        ESP_ERROR_CHECK(pcf8563_set_time(&rtc_dev, &current_time));
    }
    ESP_LOGI(TAG, "Current time: %04d-%02d-%02d %02d:%02d:%02d",
             current_time.tm_year + 1900,
             current_time.tm_mon + 1,
             current_time.tm_mday,
             current_time.tm_hour,
             current_time.tm_min,
             current_time.tm_sec);


    pcf8563_get_time(&rtc_dev, &current_time, &valid_time);
    
    
    ESP_LOGI(TAG, "Current time: %04d-%02d-%02d %02d:%02d:%02d",
             current_time.tm_year + 1900,
             current_time.tm_mon + 1,
             current_time.tm_mday,
             current_time.tm_hour,
             current_time.tm_min,
             current_time.tm_sec);


    // Display driver and LVGL init
    display_init();
    lv_init();
    lv_tick_set_cb(xTaskGetTickCount);

    lv_display_t *display = lv_display_create(LCD_H_RES, LCD_V_RES);
    static uint8_t buf[LCD_H_RES * LCD_V_RES / 20]; // Buffer size set for 240x240 display

    lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, my_flush_cb); // Set the flush callback

    // Setting up touch I2C
    memset(&chsc6x_dev, 0, sizeof(i2c_dev_t));
    chsc6x_init_desc(&chsc6x_dev, 0, I2C_MASTER_SDA_PIN, I2C_MASTER_SCL_PIN);
    touch_init();

    // Allocate memory for list of pointers to medicines
    medicines_list = heap_caps_calloc(MAX_MEDICINES_TOTAL, sizeof(medicine_t *), MALLOC_CAP_8BIT);

    // Default screen
    init_lvgl_default_scr(NULL);

    // Initialize motor driver
    init_motor();

    // Init alarm
    alarm_gpio_init();

    medicine_t* medicine_A_p = heap_caps_malloc(sizeof(medicine_t), MALLOC_CAP_8BIT);
    strcpy(medicine_A_p->name, "Medicine A");
    medicine_A_p->doses_per_day = 3;
    medicine_A_p->dose_times[0].hour = 8;   // First dose at 8 AM
    medicine_A_p->dose_times[0].minute = 0;
    medicine_A_p->dose_times[1].hour = 12;  // Second dose at 12 PM
    medicine_A_p->dose_times[1].minute = 0;
    medicine_A_p->dose_times[2].hour = 18;  // Third dose at 6 PM
    medicine_A_p->dose_times[2].minute = 0;
    strcpy(medicine_A_p->special_requirements, "Take after meals");
    medicine_A_p->treatment_start_date = current_time; // Set start date to current time
    medicine_A_p->treatment_end_date = current_time; 
    medicine_A_p->treatment_end_date.tm_mday += 7; // Set treatment end date to 7 days later
    add_medicine(medicine_A_p);

    medicine_t* medicine_B_p = heap_caps_malloc(sizeof(medicine_t), MALLOC_CAP_8BIT);
    strcpy(medicine_B_p->name, "Medicine B");
    medicine_B_p->doses_per_day = 2;
    medicine_B_p->dose_times[0].hour = 9;   // First dose at 9 AM
    medicine_B_p->dose_times[0].minute = 30;
    // set time form now + 2 minutes
    medicine_B_p->dose_times[1].hour = current_time.tm_hour;
    medicine_B_p->dose_times[1].minute = (current_time.tm_min + 40) % 60;

    strcpy(medicine_B_p->special_requirements, "Take with water");
    medicine_B_p->treatment_start_date = current_time; // Set start date to current current_time
    medicine_B_p->treatment_end_date = current_time; 
    medicine_B_p->treatment_end_date.tm_mday += 5 % 31; // Set treatment end date to 5 days later

    add_medicine(medicine_B_p);

     ESP_LOGI(TAG, "Current time: %04d-%02d-%02d %02d:%02d:%02d",
             current_time.tm_year + 1900,
             current_time.tm_mon + 1,
             current_time.tm_mday,
             current_time.tm_hour,
             current_time.tm_min,
             current_time.tm_sec);

    ESP_LOGI(TAG, "Setting up alarm timer");
    xTaskCreate(check_for_alarm_task, "Alarm check", 4096, NULL, 6, NULL);


    ESP_LOGI(TAG, "Entering LVGL task loop");
    xTaskCreate(lvgl_task, "LVGL", 8192, NULL, 5, NULL);


}