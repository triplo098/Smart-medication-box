
// LVGL screens includes
#include "lvgl_default_scr.h"
#include "lvgl_medications_list_scr.h"

// Other includes
#include "display_driver.h"
#include "motor_driver.h"
#include "medicines_managment.h"

static void show_medicine_list_btn_event_handler(lv_event_t *e);

void init_lvgl_default_scr(lv_obj_t *scr)
{
    if (scr == NULL)
    {
        scr = lv_obj_create(NULL);
    }
    lv_obj_set_size(scr, 200, 200);
    lv_obj_center(scr);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_shadow_width(scr, 0, 0);

    // Time label
    time_label = lv_label_create(scr);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_align(time_label, LV_ALIGN_TOP_MID, 0, 40);
    lv_timer_create(lvgl_update_time_cb, 5000, NULL);
    lvgl_update_time_cb(); // Initial call to set time immediately

    lv_obj_t *btn1 = lv_btn_create(scr);
    lv_obj_set_size(btn1, 160, 30);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0x195592), 0);
    lv_obj_set_style_radius(btn1, 8, 0);

    lv_obj_t *show_medicines_label = lv_label_create(btn1);
    lv_label_set_text(show_medicines_label, "Show medicine list");
    lv_obj_center(show_medicines_label);
    lv_obj_add_event_cb(btn1, show_medicine_list_btn_event_handler, LV_EVENT_ALL, NULL);

    lv_obj_t *next_reminder_label = lv_label_create(scr);
    lv_obj_align(next_reminder_label, LV_ALIGN_CENTER, 0, 40);
    lv_obj_set_style_text_color(next_reminder_label, lv_color_white(), 0);

    struct tm next_time;
    get_next_medicine_time(&next_time);

    char buff[64];
    snprintf(buff, sizeof(buff), "Next reminder time: %02d:%02d", next_time.tm_hour, next_time.tm_min);

    ESP_LOGI("LVGL_DEFAULT_SCR", "Next reminder time: %s", buff);
    lv_label_set_text(next_reminder_label, buff);

    lv_screen_load(scr);
}

static void show_medicine_list_btn_event_handler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_PRESSED)
    {
        // Code to handle button press
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0D47A1), 0); // Darker blue on press
        
        init_lvgl_medications_list_scr();
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        // Code to handle button release
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1976D2), 0); // Original color on release
    }
}
