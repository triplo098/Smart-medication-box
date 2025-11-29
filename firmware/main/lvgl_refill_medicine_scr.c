// Minimal refill screen for LVGL with empty button handlers

#include "lvgl.h"
#include "esp_log.h"
#include "medicines_managment.h" // your medicine_t definition
#include "lvgl_default_scr.h" // for colors and back button

static const char *TAG = "REFILL_SCR";

static lv_obj_t *refill_scr = NULL;
static medicine_t *current_refill_medicine = NULL;
static int current_compartment = 0;

/* Empty handlers - implement logic later */
static void back_btn_event_handler(lv_event_t *e) { (void)e; }
static void next_compartment_btn_event_handler(lv_event_t *e) { (void)e; }
static void prev_compartment_btn_event_handler(lv_event_t *e) { (void)e; }
static void accept_btn_event_handler(lv_event_t *e) { (void)e; }

static void add_back_btn(lv_obj_t *scr);

void init_refill_medicine_scr(medicine_t *medicine)
{
    ESP_LOGI(TAG, "init_refill_medicine_scr (minimal)");

    if (!medicine) {
        ESP_LOGW(TAG, "no medicine provided");
        return;
    }

    current_refill_medicine = medicine;
    current_compartment = 0;

    log_medicine_info(current_refill_medicine);
    /* Create screen */
    refill_scr = lv_obj_create(NULL);
    lv_obj_set_size(refill_scr, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(refill_scr, LVGL_BLACK_COLOR, 0);

    /* Top medicine name */
    lv_obj_t *name_label = lv_label_create(refill_scr);
    lv_label_set_text_fmt(name_label, "%s", current_refill_medicine->name);
    lv_obj_set_style_text_color(name_label, LVGL_WHITE_COLOR, 0);
    //set label background to dark blue
    lv_obj_set_style_bg_color(name_label, LVGL_DARK_BLUE_COLOR, 0);


    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 20);




    /* Instruction / compartment label */
    lv_obj_t *instr_label = lv_label_create(refill_scr);
    lv_label_set_text_fmt(instr_label,
        "Add dose:\nCell %d", current_compartment + 1);
    lv_obj_set_style_text_color(instr_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(instr_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(instr_label, LV_ALIGN_CENTER, 0, -10);

    //Previous button
    lv_obj_t *accept_btn = lv_btn_create(refill_scr);
    lv_obj_set_size(accept_btn, 50, 50);
    lv_obj_align(accept_btn, LV_ALIGN_BOTTOM_MID, -40, -25);
    lv_obj_set_style_radius(accept_btn, 8, 0);
    lv_obj_t *accept_label = lv_label_create(accept_btn);
    lv_label_set_text(accept_label, LV_SYMBOL_LEFT);
    lv_obj_center(accept_label);
    lv_obj_add_event_cb(accept_btn, accept_btn_event_handler, LV_EVENT_CLICKED, NULL);

    // Next button
    lv_obj_t *next_btn = lv_btn_create(refill_scr);
    lv_obj_set_size(next_btn, 50, 50);
    lv_obj_align(next_btn, LV_ALIGN_BOTTOM_MID, 40, -25);
    lv_obj_set_style_radius(next_btn, 8, 0);
    lv_obj_t *next_label = lv_label_create(next_btn);
    lv_label_set_text(next_label, LV_SYMBOL_RIGHT);
    lv_obj_center(next_label);
    lv_obj_add_event_cb(next_btn, next_compartment_btn_event_handler, LV_EVENT_CLICKED, NULL);



    /* Load screen */
    lv_scr_load(refill_scr);
    ESP_LOGI(TAG, "screen loaded (minimal)");
}

static void add_back_btn(lv_obj_t *scr)
{
    lv_obj_t *back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 30, 30);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, -25, -15);
    lv_obj_set_style_radius(back_btn, 8, 0);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, back_btn_event_handler, LV_EVENT_ALL, NULL);
}