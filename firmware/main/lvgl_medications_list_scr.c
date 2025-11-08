#include "display_driver.h"
#include "lvgl_default_scr.h"

static void back_to_start_btn_event_handler(lv_event_t *e);


void init_lvgl_medications_list_scr()
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Medicine List Screen");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_center(label);

    // Add back button
    lv_obj_t *back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 80, 30);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x1976D2), 0);
    lv_obj_set_style_radius(back_btn, 8, 0);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);

    lv_obj_add_event_cb(back_btn, back_to_start_btn_event_handler, LV_EVENT_ALL, NULL);

    lv_screen_load(scr);
}

static void back_to_start_btn_event_handler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_PRESSED)
    {
        // Code to handle button press
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0D47A1), 0); // Darker blue on press

        init_lvgl_default_scr(NULL);
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1976D2), 0); // Original color on release
    }
}