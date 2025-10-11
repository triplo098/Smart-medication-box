#include "display_driver.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_gc9a01.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "lvgl.h"
#include "pcf8563.h"

// 0x2e
#define CHSC6X_I2C_ID 44
#define CHSC6X_READ_POINT_LEN 5

static const char *TAG = "display_driver";
esp_lcd_panel_handle_t panel_handle = NULL;
uint8_t screen_rotation = 0;

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

    ESP_LOGI(TAG, "Install GC9A01 panel driver");

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,  // Set to -1 if not use
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // RGB element order: R-G-B
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    esp_lcd_panel_mirror(panel_handle, true, false);

    screen_rotation = 0;

    ESP_LOGI(TAG, "Display initialized");
}

void my_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    // ESP_LOGI(TAG, "Flushing area x1:%d, y1:%d, x2:%d, y2:%d", area->x1, area->y1, area->x2, area->y2);

    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // because SPI LCD is big-endian, we need to swap the RGB bytes order

    // Assures colors are correct on little-endian systems
    lv_draw_sw_rgb565_swap(px_map, (offsetx2 + 1 - offsetx1) * (offsety2 + 1 - offsety1));

    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);

    lv_display_flush_ready(display);
}

void lvgl_update_time_cb()
{
    struct tm time;
    bool valid;

    if (pcf8563_get_time(&rtc_dev, &time, &valid) == ESP_OK && valid)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%02d:%02d %02d/%02d/%04d",
                 time.tm_hour, time.tm_min, time.tm_mday, time.tm_mon + 1, time.tm_year + 1900);

        lv_label_set_text(time_label, buf);
    }
}

esp_err_t chsc6x_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    dev->port = port;
    dev->addr = CHSC6X_I2C_ADDR;
    dev->cfg.sda_io_num = sda_gpio;
    dev->cfg.scl_io_num = scl_gpio;
    return i2c_dev_create_mutex(dev);
}

void chsc6x_convert_xy(uint8_t *x, uint8_t *y)
{
    uint8_t x_tmp = *x, y_tmp = *y, _end = 0;
    for (int i = 1; i <= screen_rotation; i++)
    {
        x_tmp = *x;
        y_tmp = *y;
        _end = (i % 2) ? EXAMPLE_LCD_V_RES : EXAMPLE_LCD_H_RES;
        *x = y_tmp;
        *y = _end - x_tmp;
    }
}

bool chsc6x_is_pressed(void)
{
    if (gpio_get_level(TOUCH_INT) != 0)
        return false;
    {
        // vTaskDelay(pdMS_TO_TICKS(5));
        if (gpio_get_level(TOUCH_INT) != 0)
            return false;
    }
    return true;
}

void chsc6x_get_xy(lv_coord_t *x, lv_coord_t *y)
{
    uint8_t temp[CHSC6X_READ_POINT_LEN] = {0};

    // Perform I2C read
    esp_err_t res = i2c_dev_read(&chsc6x_dev, NULL, 0, temp, CHSC6X_READ_POINT_LEN);
    if (res != ESP_OK)
    {
        ESP_LOGW(TAG, "I2C read failed: %s", esp_err_to_name(res));
        *x = -1;
        *y = -1;
        return;
    }

    // Verify valid touch data
    if (temp[0] == 0x01)
    {
        chsc6x_convert_xy(&temp[2], &temp[4]);
        *x = temp[2];
        *y = temp[4];
    }
    else
    {
        *x = -1;
        *y = -1;
    }
}

void chsc6x_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    lv_coord_t touchX, touchY;
    if (!chsc6x_is_pressed())
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;
        chsc6x_get_xy(&touchX, &touchY);
        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;
        ESP_LOGI(TAG, "Touch at x: %d, y: %d", data->point.x, data->point.y);
    }
}

void touch_init()
{

    indev = lv_indev_create();
    ESP_LOGI(TAG, "Touch I2C Init");

    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, chsc6x_read);
}