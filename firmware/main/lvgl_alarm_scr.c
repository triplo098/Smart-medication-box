// Minimal alarm screen for LVGL (visual only, empty handlers)
// Uses the default/back patterns from your example.
// Drop into your project and call init_lvgl_alarm_scr() to show it.

#include "lvgl.h"
#include "esp_log.h"

#include "lvgl_default_scr.h"        // for init_lvgl_default_scr()
#include "lvgl_medicines_list_scr.h" // optional navigation target
#include "medicines_managment.h"     // medicine_t if you want to build buttons from real data

static const char *TAG = "LVGL_ALARM_SCR";

/* Forward declarations */
static void back_btn_event_handler(lv_event_t *e);
static void alarm_btn_event_handler(lv_event_t *e);
static void medicine_btn_event_handler(lv_event_t *e);
static void add_back_btn(lv_obj_t *parent);


extern struct tm current_time;


void init_lvgl_alarm_scr()
{
    ESP_LOGI(TAG, "init_lvgl_alarm_scr");

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_set_style_bg_color(scr, LVGL_BLACK_COLOR, 0); // black background for whole display


    lv_obj_set_style_bg_color(scr, LVGL_BLACK_COLOR, 0); // dark grey
    /* Title: "Lista leków:" */
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Medicines list:");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

    /* A small vertical container for medicine buttons (flex column) */
    lv_obj_t *list = lv_obj_create(scr);
    lv_obj_set_size(list, 140, 120); // relative size
    lv_obj_center(list);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list, 8, 0);
    lv_obj_set_style_pad_column(list, 0, 0);
    lv_obj_set_style_pad_left(list, 0, 0);
    lv_obj_set_style_pad_right(list, 0, 0);
    lv_obj_set_style_pad_top(list, 0, 0);
    lv_obj_set_style_pad_bottom(list, 0, 0);
    lv_obj_set_style_border_width(list, 0, 0); // Remove frame

    medicine_t **meds = NULL;
    size_t med_count = 0;
    get_medicines_list(&meds, &med_count);

    if (med_count == 0 || meds == NULL) {
        lv_obj_t *no_label = lv_label_create(list);
        lv_label_set_text(no_label, "Brak leków");
        lv_obj_set_style_text_color(no_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_center(no_label);
    } else {
        /* Create a button for each non-NULL entry in the global array until we've created med_count buttons. */
        size_t created = 0;
        for (size_t i = 0; created < med_count && i < MAX_MEDICINES_TOTAL; ++i) {
            if (meds[i] == NULL) continue;

            lv_obj_t *mbtn = lv_btn_create(list);
            lv_obj_set_size(mbtn, lv_pct(100), 32);
            lv_obj_set_style_bg_color(mbtn, lv_color_hex(0x0D47A1), 0); // blue
            lv_obj_set_style_radius(mbtn, 16, 0);

            lv_obj_t *mlab = lv_label_create(mbtn);
            /* Use medicine name if available, otherwise fallback */
            if (meds[i]->name[0] != '\0') {
                lv_label_set_text(mlab, meds[i]->name);
            } else {
                char tmp[32];
                snprintf(tmp, sizeof(tmp), "Lek %u", (unsigned)(created + 1));
                lv_label_set_text(mlab, tmp);
            }
            lv_obj_center(mlab);

            /* Pass the medicine pointer as user_data to the handler */
            lv_obj_add_event_cb(mbtn, medicine_btn_event_handler, LV_EVENT_CLICKED, (void*)meds[i]);

            ++created;
        }
    }
    /* Alarm large red button */
    lv_obj_t *alarm_btn = lv_btn_create(scr);
    lv_obj_set_size(alarm_btn, 90, 45);
    lv_obj_align(alarm_btn, LV_ALIGN_BOTTOM_MID, 0 , -30); // below the list
    lv_obj_set_style_bg_color(alarm_btn, lv_color_hex(0x8B0000), 0); // dark red
    lv_obj_set_style_radius(alarm_btn, 12, 0);

    /* Alarm label with time and action (two lines) */
    lv_obj_t *alarm_label = lv_label_create(alarm_btn);

    
    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", current_time.tm_hour, current_time.tm_min);

    lv_label_set_text_fmt(alarm_label, "%s\nSTOP", time_str);

    
    lv_obj_set_style_text_align(alarm_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(alarm_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(alarm_label);

    /* Empty handler; wire esp/system logic here later */
    lv_obj_add_event_cb(alarm_btn, alarm_btn_event_handler, LV_EVENT_ALL, NULL);


    /* Load the screen */
    lv_scr_load(scr);
}

/* --- Empty / placeholder handlers --- */
static void medicine_btn_event_handler(lv_event_t *e)
{
    /* Intentionally minimal: replace with real logic (open medicine, toggle selection, etc.) */
    (void)e;
}

static void alarm_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_PRESSED) {
        /* visual pressed feedback */
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x660000), 0);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x8B0000), 0);
    } else if (code == LV_EVENT_CLICKED) {
        /* action placeholder: stop alarm / notify system */
        ESP_LOGI(TAG, "Alarm button clicked (placeholder)");
        init_lvgl_default_scr(NULL);
    }
}

static void back_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_PRESSED) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0D47A1), 0);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1976D2), 0);
    } else if (code == LV_EVENT_CLICKED) {
        /* Navigate back to default screen (placeholder) */
        ESP_LOGI(TAG, "Back pressed - returning to default screen");
        init_lvgl_default_scr(NULL);
    }
}

static void add_back_btn(lv_obj_t *parent)
{
    lv_obj_t *back_btn = lv_btn_create(parent);
    lv_obj_set_size(back_btn, 30, 30);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, -25, -15);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x0D47A1), 0);
    lv_obj_set_style_radius(back_btn, 8, 0);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, back_btn_event_handler, LV_EVENT_ALL, NULL);
}