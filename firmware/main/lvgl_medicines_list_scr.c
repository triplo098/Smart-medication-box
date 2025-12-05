#include "lvgl_medicines_list_scr.h"
#include "lvgl_default_scr.h"
#include "lvgl_add_medicine_scr.h"

#include "display_driver.h"
#include "medicines_managment.h"
#include "esp_log.h"

static const char *TAG = "LVGL_MED_LIST_SCR";

static lv_obj_t *create_medicine_entry(lv_obj_t *parent, const char *name);
extern medicine_t **medicines_list;

static void medicine_name_btn_event_handle(lv_event_t *e);
static void add_medicine_btn_event_handle(lv_event_t *e);

static void add_back_btn(lv_obj_t *scr);
static void back_btn_event_handler(lv_event_t *e);


static  lv_obj_t *scr;

static void remove_medicine_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    const char *med_name = (const char *)lv_event_get_user_data(e);

    ESP_LOGI(TAG, "Remove medicine button clicked for medicine: %s", med_name);

    remove_medicine(med_name);

    // Refresh the medicines list screen
    init_lvgl_medicines_list_scr();

}

void init_lvgl_medicines_list_scr()
{
    ESP_LOGI(TAG, "Initializing medicines list screen");

    scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, LCD_H_RES, LCD_V_RES);
    lv_obj_set_style_bg_color(scr, LVGL_BLACK_COLOR, 0);
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);

    lv_obj_t *list = lv_list_create(scr);
    lv_obj_set_size(list, 190, 140);
    lv_obj_set_style_bg_color(list, LVGL_BLACK_COLOR, 0);

    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 25);
    lv_obj_set_scroll_dir(list, LV_DIR_VER); // allow vertical scroll only
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_ON);
    lv_obj_set_style_border_width(list, 0, 0);

    for (uint16_t i = 0; i < MAX_MEDICINES_TOTAL; i++)
    {
        if (medicines_list[i] == NULL)
        {
            continue; // Skip empty slots
        }
        create_medicine_entry(list, medicines_list[i]->name);
    }

    ESP_LOGI(TAG, "medicines list populated");

    // Add back button
    add_back_btn(scr);

    // Add add medicine button
    lv_obj_t *add_btn = lv_btn_create(scr);
    lv_obj_set_size(add_btn, 30, 30);
    lv_obj_align(add_btn, LV_ALIGN_BOTTOM_MID, 25, -15);

    lv_obj_set_style_bg_color(add_btn, LVGL_DARK_BLUE_COLOR, 0);
    lv_obj_set_style_radius(add_btn, 8, 0);
    lv_obj_t *add_label = lv_label_create(add_btn);
    lv_label_set_text(add_label, LV_SYMBOL_PLUS);
    lv_obj_center(add_label);
    lv_obj_add_event_cb(add_btn, add_medicine_btn_event_handle, LV_EVENT_CLICKED, NULL);

    lv_screen_load(scr);
}

static void add_medicine_btn_event_handle(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    // Code to handle button press
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x0D47A1), 0); // Darker blue on press
    
    init_lvgl_add_medicine_scr(NULL);

    lv_obj_set_style_bg_color(btn, lv_color_hex(0x1976D2), 0); // Original color on release
}

static void medicine_name_btn_event_handle(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    const char *med_name = (const char *)lv_event_get_user_data(e);

    if (code == LV_EVENT_PRESSED || code == LV_EVENT_CLICKED || code == LV_EVENT_SHORT_CLICKED)
    {
        // Code to handle button press
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0D47A1), 0); // Darker blue on press

        medicine_t *med = get_medicine_from_name(med_name);
        init_lvgl_add_medicine_scr(med);
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1976D2), 0); // Original color on release
    }
}

static lv_obj_t *create_medicine_entry(lv_obj_t *parent, const char *name)
{
    lv_obj_t *item = lv_obj_create(parent);
    lv_obj_set_size(item, lv_pct(100), 40);
    lv_obj_set_style_radius(item, 8, 0);
    lv_obj_set_style_pad_all(item, 4, 0);
    lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_scrollbar_mode(item, LV_SCROLLBAR_MODE_OFF);

    // Left: Medicine button
    lv_obj_t *med_btn = lv_btn_create(item);
    lv_obj_set_size(med_btn, lv_pct(75), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(med_btn, LVGL_DARK_BLUE_COLOR, 0);
    lv_obj_set_style_radius(med_btn, 8, 0);

    lv_obj_add_event_cb(med_btn, medicine_name_btn_event_handle, LV_EVENT_CLICKED, (void *)name);

    lv_obj_t *med_label = lv_label_create(med_btn);
    lv_label_set_text(med_label, name);
    lv_obj_set_style_text_color(med_label, LVGL_BLACK_COLOR, 0);
    lv_obj_center(med_label);

    // Right: Delete button
    lv_obj_t *del_btn = lv_btn_create(item);
    lv_obj_set_size(del_btn, lv_pct(18), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(del_btn, LVGL_DARK_RED_COLOR, 0);
    lv_obj_set_style_radius(del_btn, 14, 0);
    lv_obj_align(del_btn, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_add_event_cb(del_btn, remove_medicine_btn_event_handler, LV_EVENT_CLICKED, (void *)name);

    lv_obj_t *x_label = lv_label_create(del_btn);
    lv_label_set_text(x_label, "x");
    lv_obj_set_style_text_color(x_label, LVGL_BLACK_COLOR, 0);
    lv_obj_center(x_label);

    return item;
}

static void back_btn_event_handler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    // Code to handle button press
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x0D47A1), 0); // Darker blue on press

    init_lvgl_default_scr(NULL);
}

static void add_back_btn(lv_obj_t *scr)
{
    lv_obj_t *back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 30, 30);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, -25, -15);
    lv_obj_set_style_bg_color(back_btn, LVGL_DARK_BLUE_COLOR, 0);
    lv_obj_set_style_radius(back_btn, 8, 0);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, back_btn_event_handler, LV_EVENT_CLICKED, NULL);
}
