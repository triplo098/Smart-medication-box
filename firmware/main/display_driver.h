#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include "lvgl.h"
#include "pcf8563.h"

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_SCLK 7
#define EXAMPLE_PIN_NUM_MOSI 9
#define EXAMPLE_PIN_NUM_MISO 8 // Not used for round display
#define EXAMPLE_PIN_NUM_LCD_DC 4
#define EXAMPLE_PIN_NUM_LCD_RST 1
#define EXAMPLE_PIN_NUM_LCD_CS 2
#define EXAMPLE_PIN_NUM_BK_LIGHT 43
#define EXAMPLE_PIN_NUM_TOUCH_CS 44 // Touch interrupt pin

// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 240 // Round display is 240x240

// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS 8
#define EXAMPLE_LCD_PARAM_BITS 8
#define EXAMPLE_LVGL_DRAW_BUF_LINES 10 // Reduced for 240x240 display
#define EXAMPLE_LVGL_TICK_PERIOD_MS 2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1000 / CONFIG_FREERTOS_HZ
#define EXAMPLE_LVGL_TASK_STACK_SIZE (4 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY 2
#define TOUCH_INT 44

#define CHSC6X_I2C_ADDR 0x2e


extern lv_obj_t *time_label;
extern lv_indev_t *indev;

extern i2c_dev_t rtc_dev;
extern i2c_dev_t chsc6x_dev;


void display_init(void);

void my_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map);

void lvgl_update_time_cb(lv_timer_t *timer);

void touch_init();

esp_err_t chsc6x_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);


#endif // DISPLAY_DRIVER_H
