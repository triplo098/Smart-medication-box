// refill_screen.c
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "lvgl.h"
#include "esp_log.h"

#include "medicines_managment.h"   // medicine_t, get_next_medicine_time_from_medicine(), etc.
#include "sections_controller.h"
#include "lvgl_default_scr.h"      // for init_lvgl_default_scr() or similar (used on back)

static const char *TAG = "REFILL_SCR";

/* Screen objects and state */
static lv_obj_t *refill_scr = NULL;
static lv_obj_t *name_label = NULL;
static lv_obj_t *instr_label = NULL;
static lv_obj_t *dose_label = NULL;
static lv_obj_t *accept_btn = NULL;
static lv_obj_t *next_btn = NULL;
static lv_obj_t *prev_btn = NULL;
static lv_obj_t *back_btn = NULL;

static medicine_t *current_refill_medicine = NULL;


extern unsigned int current_section; /* if you use this elsewhere */


struct tm times_array[MAX_MEDICINE_DOSES_NUMBER_PER_DAY * NUMEBR_OF_SECTIONS];
size_t times_count = 0;
uint16_t times_index = 0;
extern section sections[NUMEBR_OF_SECTIONS];

/* Forward declarations */
static void update_dose_label(void);
static void next_btn_event_handler(lv_event_t *e);
static void prev_btn_event_handler(lv_event_t *e);

/* Utility: safe color macros (you can replace with your project macros) */
#define BG_COLOR      lv_color_hex(0x000000) /* black */
#define TEXT_COLOR    lv_color_hex(0xFFFFFF) /* white */
#define ACCENT_COLOR  lv_color_hex(0x0A3D62) /* dark blue */

/**
 * Initialize refill screen for a given medicine.
 * This will create the LVGL screen, labels and buttons, and show it.
 */
void init_refill_medicine_scr(medicine_t *medicine)
{
    ESP_LOGI(TAG, "init_refill_medicine_scr");

    if (!medicine) {
        ESP_LOGW(TAG, "No medicine provided, aborting screen init");
        return;
    }

    current_refill_medicine = medicine;

    get_all_medicine_time(times_array, &times_count, medicine);

    for(size_t i = 0; i < times_count; i++) {
        ESP_LOGI(TAG, "Medicine dose time %zu: %04d-%02d-%02d %02d:%02d",
                 i,
                 times_array[i].tm_year + 1900,
                 times_array[i].tm_mon + 1,
                 times_array[i].tm_mday,
                 times_array[i].tm_hour,
                 times_array[i].tm_min);
    }
    
    uint8_t sec_num = get_next_target_section_number_for_time(times_array[times_index]);
    set_section(sec_num);

    /* Create screen root */
    if (refill_scr) {
        lv_obj_del(refill_scr);
        refill_scr = NULL;
    }
    refill_scr = lv_obj_create(NULL);
    lv_obj_set_size(refill_scr, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(refill_scr, BG_COLOR, 0);

    /* Top: Medicine name */
    name_label = lv_label_create(refill_scr);
    lv_label_set_text_fmt(name_label, "%s", current_refill_medicine->name);
    lv_obj_set_style_text_color(name_label, TEXT_COLOR, 0);
    lv_obj_set_style_bg_color(name_label, ACCENT_COLOR, 0);
    lv_obj_set_style_pad_ver(name_label, 6, 0);
    lv_obj_set_style_pad_left(name_label, 8, 0);
    lv_obj_set_style_pad_right(name_label, 8, 0);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 14);

    /* Instruction (compartment index / optional next time) */
    instr_label = lv_label_create(refill_scr);
    lv_obj_set_style_text_color(instr_label, TEXT_COLOR, 0);
    lv_obj_set_style_text_align(instr_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(instr_label, ""); /* filled below */
    lv_obj_align(instr_label, LV_ALIGN_CENTER, 0, -18);

    /* Dose label (time) */
    dose_label = lv_label_create(refill_scr);
    lv_obj_set_style_text_color(dose_label, TEXT_COLOR, 0);
    lv_label_set_text(dose_label, ""); /* updated */
    lv_obj_align(dose_label, LV_ALIGN_CENTER, 0, 24);

    /* Prev button (left) */
    prev_btn = lv_btn_create(refill_scr);
    lv_obj_set_size(prev_btn, 50, 50);
    lv_obj_align(prev_btn, LV_ALIGN_BOTTOM_LEFT, 40, -40);
    lv_obj_set_style_radius(prev_btn, 8, 0);
    lv_obj_t *prev_label = lv_label_create(prev_btn);
    lv_label_set_text(prev_label, LV_SYMBOL_LEFT);
    lv_obj_center(prev_label);
    lv_obj_add_event_cb(prev_btn, prev_btn_event_handler, LV_EVENT_CLICKED, NULL);

    /* Next button (right) */
    next_btn = lv_btn_create(refill_scr);
    lv_obj_set_size(next_btn, 50, 50);
    lv_obj_align(next_btn, LV_ALIGN_BOTTOM_RIGHT, -40, -40);
    lv_obj_set_style_radius(next_btn, 8, 0);
    lv_obj_t *next_label = lv_label_create(next_btn);
    lv_label_set_text(next_label, LV_SYMBOL_RIGHT);
    lv_obj_center(next_label);
    lv_obj_add_event_cb(next_btn, next_btn_event_handler, LV_EVENT_CLICKED, NULL);

    /* Populate labels with initial values */
    update_dose_label();

    /* Load the screen */
    lv_scr_load(refill_scr);
    ESP_LOGI(TAG, "Refill screen loaded for '%s'", current_refill_medicine->name);
}

/* Helper: update instr_label and dose_label from current_refill_medicine & current_dose_index */
static void update_dose_label(void)
{
    if (!current_refill_medicine) {
        ESP_LOGW(TAG, "update_dose_label: no medicine");
        return;
    }

    if (current_refill_medicine->doses_per_day == 0) {
        lv_label_set_text(instr_label, "No doses configured");
        lv_label_set_text(dose_label, "");
        return;
    }


    /* Update instr_label */
    char buf[128];
    snprintf(buf, sizeof(buf), "Filled %u of %u\nCurrent section: %u", times_index, times_count, current_section);

    lv_label_set_text(instr_label, buf);
    /* Update dose_label */
    if (times_index >= times_count) {
        lv_label_set_text(dose_label, "All doses filled");
        
        init_lvgl_default_scr(NULL);
        return;
    }
    // Addin dose at times_index with full date
    struct tm *dose_time = &times_array[times_index];
    snprintf(buf, sizeof(buf), "Adding dose for: %02d.%02d.%04d %02d:%02d",
             dose_time->tm_mday,
             dose_time->tm_mon + 1,
             dose_time->tm_year + 1900,
             dose_time->tm_hour,
             dose_time->tm_min);

    lv_label_set_text(dose_label, buf);
}

/* Event handlers -----------------------------------------------------------*/


static void next_btn_event_handler(lv_event_t *e)
{
    (void)e;
    if (!current_refill_medicine) {
        ESP_LOGW(TAG, "Next: no current medicine");
        return;
    }

    int doses = (int)current_refill_medicine->doses_per_day;
    if (doses <= 0) {
        ESP_LOGW(TAG, "Next: no doses configured");
        return;
    }

    if(times_index >= times_count) {
        ESP_LOGI(TAG, "Next: all doses filled");
        
        lv_obj_delete(lv_screen_active());
        init_lvgl_default_scr(NULL);
    }

    sections[current_section].filled = true;
    sections[current_section].exact_time = times_array[times_index];
    // Update state
    times_index++;
    uint8_t sec_num = get_next_target_section_number_for_time(times_array[times_index]);

    ESP_LOGI(TAG, "Next: Moving to section %d for dose index %u", sec_num, times_index);
    set_section(sec_num);

    update_dose_label();
}

static void prev_btn_event_handler(lv_event_t *e)
{
    (void)e;
    if (!current_refill_medicine) {
        ESP_LOGW(TAG, "Prev: no current medicine");
        return;
    }

    init_lvgl_default_scr(NULL);
}

/* Optional: a cleanup function you can call if you want to free the screen */
void deinit_refill_medicine_scr(void)
{
    if (refill_scr) {
        lv_obj_del(refill_scr);
        refill_scr = NULL;
    }
    current_refill_medicine = NULL;
}
