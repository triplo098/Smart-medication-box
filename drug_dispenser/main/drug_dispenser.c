void display_init(void);

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_gc9a01.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_ops.h"

#define TAG "display_example"

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_SCLK 8
#define EXAMPLE_PIN_NUM_MOSI 10
#define EXAMPLE_PIN_NUM_MISO -1 // Not used for round display
#define EXAMPLE_PIN_NUM_LCD_DC 3
#define EXAMPLE_PIN_NUM_LCD_RST 2
#define EXAMPLE_PIN_NUM_LCD_CS 1
#define EXAMPLE_PIN_NUM_BK_LIGHT 6
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

esp_lcd_panel_handle_t panel_handle = NULL;

void display_init(void)
{
    esp_lcd_panel_io_handle_t io_handle = NULL;
    ESP_LOGI(TAG, "Initialize SPI bus");
    const spi_bus_config_t bus_config = GC9A01_PANEL_BUS_SPI_CONFIG(EXAMPLE_PIN_NUM_SCLK, EXAMPLE_PIN_NUM_MOSI,
                                                                    EXAMPLE_LCD_H_RES * 80 * sizeof(uint16_t));
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = GC9A01_PANEL_IO_SPI_CONFIG(EXAMPLE_PIN_NUM_LCD_CS, EXAMPLE_PIN_NUM_LCD_DC,
                                                                               NULL, NULL);
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));

    /**
     * Uncomment these lines if use custom initialization commands.
     * The array should be declared as "static const" and positioned outside the function.
     */
    // static const gc9a01_lcd_init_cmd_t lcd_init_cmds[] = {
    // //  {cmd, { data }, data_size, delay_ms}
    //     {0xfe, (uint8_t []){0x00}, 0, 0},
    //     {0xef, (uint8_t []){0x00}, 0, 0},
    //     {0xeb, (uint8_t []){0x14}, 1, 0},
    //     ...
    // };

    ESP_LOGI(TAG, "Install GC9A01 panel driver");
    // gc9a01_vendor_config_t vendor_config = {  // Uncomment these lines if use custom initialization commands
    //     .init_cmds = lcd_init_cmds,
    //     .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(gc9a01_lcd_init_cmd_t),
    // };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,  // Set to -1 if not use
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // RGB element order: R-G-B
        .bits_per_pixel = 16,                       // Implemented by LCD command `3Ah` (16/18)
        // .vendor_config = &vendor_config,          // Uncomment this line if use custom initialization commands
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // // Configure backlight
    // if (EXAMPLE_PIN_NUM_BK_LIGHT >= 0)
    // {
    //     ESP_LOGI(TAG, "Turn on LCD backlight");
    //     gpio_config_t bk_gpio_config = {
    //         .mode = GPIO_MODE_OUTPUT,
    //         .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT};
    //     ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    //     gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
    // }


    ESP_LOGI(TAG, "Display initialized");
}

void my_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{   
    ESP_LOGI(TAG, "Flushing area x1:%d, y1:%d, x2:%d, y2:%d", area->x1, area->y1, area->x2, area->y2);

    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // because SPI LCD is big-endian, we need to swap the RGB bytes order
    lv_draw_sw_rgb565_swap(px_map, (offsetx2 + 1 - offsetx1) * (offsety2 + 1 - offsety1));
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);

    lv_display_flush_ready(display);
}

void app_main(void)
{
    display_init();

    lv_init();
    ESP_LOGI(TAG, "LVGL initialized");


    lv_tick_set_cb(xTaskGetTickCount);
    ESP_LOGI(TAG, "LVGL tick set");

    lv_display_t *display = lv_display_create(EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);
    ESP_LOGI(TAG, "LVGL display created");


    static uint8_t buf[EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES / 10 * 2]; // Reduced buffer size for 240x240 display

    lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    ESP_LOGI(TAG, "LVGL display buffers set");

    /* This callback will display the rendered image */
    ESP_LOGI(TAG, "Setting flusb_cb");
    lv_display_set_flush_cb(display, my_flush_cb);

    /* Create widgets */
    lv_obj_t *my_button1 = lv_button_create(lv_screen_active());
    /* Set parent-sized width, and content-sized height */
    lv_obj_set_size(my_button1, lv_pct(100), LV_SIZE_CONTENT);
    /* Align to the right center with 20px offset horizontally */
    lv_obj_align(my_button1, LV_ALIGN_RIGHT_MID, -20, 0);

    /* Create widgets */
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello LVGL!");


    ESP_LOGI(TAG, "Entering main loop");


    while (1)
    {
        ESP_LOGI(TAG, "spin");
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}