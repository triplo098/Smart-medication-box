#include "lvgl_add_medicine_scr.h"
#include "lvgl_medicines_list_scr.h"
#include "lvgl_default_scr.h"
#include "motor_driver.h"
#include "alarm_helpers.h"
// #include "lvgl_keyboard_scr.h"
// #include "lvgl_calendar_scr.h"
// #include "lvgl_time_picker_scr.h"
#include "medicines_managment.h"
#include "display_driver.h"

#include "esp_log.h"
#include <string.h>
#include <stdio.h>

extern struct tm current_time;


static const char *TAG = "scr";

// Screen objects
static lv_obj_t *scr = NULL;

static lv_obj_t *name_btn = NULL;
static lv_obj_t *name_label = NULL;

static lv_obj_t *frequency_btn = NULL;
static lv_obj_t *frequency_label = NULL;

static lv_obj_t *start_date_btn = NULL;
static lv_obj_t *start_date_label = NULL;

static lv_obj_t *end_date_btn = NULL;
static lv_obj_t *end_date_label = NULL;

static lv_obj_t *special_req_btn = NULL;
static lv_obj_t *special_req_label = NULL;

// Medicine data
static medicine_t new_medicine;

// Event handlers declarations
static void name_btn_event_handler(lv_event_t *e);
static void frequency_btn_event_handler(lv_event_t *e);
static void start_date_btn_event_handler(lv_event_t *e);
static void end_date_btn_event_handler(lv_event_t *e);
static void back_btn_event_handler(lv_event_t *e);
static void special_req_btn_event_handler(lv_event_t *e);


void init_lvgl_add_medicine_scr(medicine_t* medicine)
{   

    ESP_LOGI(TAG, "Initializing add medicine screen");

    scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, LCD_H_RES, LCD_V_RES);
    lv_obj_set_style_bg_color(scr, LVGL_BLACK_COLOR, 0);
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);


    // Initialize medicine structure
    if(medicine == NULL) {
        medicine = heap_caps_malloc(sizeof(medicine_t), MALLOC_CAP_8BIT);
        strcpy(medicine->name, "Medicine name");
        medicine->doses_per_day = 1;
        medicine->dose_times[0].hour = 8; 
        medicine->dose_times[0].minute = 0;
        medicine->treatment_start_date = current_time;
        medicine->treatment_end_date = current_time;

        medicine->treatment_end_date.tm_mday += 1 % 31;
        mktime(&medicine->treatment_end_date); // Normalize the date
        strcpy(medicine->special_requirements, "Special requirements: none. Tap to edit.");
    }

    ESP_LOGI(TAG, "Current time: %04d-%02d-%02d %02d:%02d:%02d",
             current_time.tm_year + 1900,
             current_time.tm_mon + 1,
             current_time.tm_mday,
             current_time.tm_hour,
             current_time.tm_min,
             current_time.tm_sec);

    
    // Name button
    name_btn = lv_btn_create(scr);
    lv_obj_set_size(name_btn, lv_pct(55), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(name_btn, LVGL_WHITE_COLOR, 0);
    lv_obj_set_style_radius(name_btn, 8, 0);
    lv_obj_align(name_btn, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_add_event_cb(name_btn, name_btn_event_handler, LV_EVENT_ALL, NULL);

    name_label = lv_label_create(name_btn);
    lv_label_set_text(name_label, medicine->name);
    lv_obj_set_style_text_color(name_label, LVGL_BLACK_COLOR, 0);
    lv_obj_center(name_label);

    // Frequency button
    frequency_btn = lv_btn_create(scr);
    lv_obj_set_size(frequency_btn, lv_pct(70), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(frequency_btn, LVGL_WHITE_COLOR, 0);
    lv_obj_set_style_radius(frequency_btn, 8, 0);
    lv_obj_align(frequency_btn, LV_ALIGN_TOP_MID, 0, 55);
    lv_obj_add_event_cb(frequency_btn, frequency_btn_event_handler, LV_EVENT_ALL, NULL);
    
    frequency_label = lv_label_create(frequency_btn);
    char freq_text[32];
    snprintf(freq_text, sizeof(freq_text), "%d daily", medicine->doses_per_day);
    lv_label_set_text(frequency_label, freq_text);
    lv_obj_set_style_text_color(frequency_label, LVGL_BLACK_COLOR, 0);
    lv_obj_center(frequency_label);

    // Start date button
    start_date_btn = lv_btn_create(scr);
    lv_obj_set_size(start_date_btn, lv_pct(70), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(start_date_btn, LVGL_WHITE_COLOR, 0);
    lv_obj_set_style_radius(start_date_btn, 8, 0);
    lv_obj_align(start_date_btn, LV_ALIGN_TOP_MID, 0, 90);
    lv_obj_add_event_cb(start_date_btn, start_date_btn_event_handler, LV_EVENT_ALL, NULL);

    start_date_label = lv_label_create(start_date_btn);
    char start_date_text[MAX_MEDICINE_STRING_LENGTH];
    snprintf(start_date_text, sizeof(start_date_text), "Start: %02d.%02d.%04d", 
             medicine->treatment_start_date.tm_mday,
             medicine->treatment_start_date.tm_mon + 1, 
             medicine->treatment_start_date.tm_year + 1900);

    lv_label_set_text(start_date_label, start_date_text);
    lv_obj_set_style_text_color(start_date_label, LVGL_BLACK_COLOR, 0);
    lv_obj_center(start_date_label);

    // End date button
    end_date_btn = lv_btn_create(scr);
    lv_obj_set_size(end_date_btn, lv_pct(70), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(end_date_btn, LVGL_WHITE_COLOR, 0);
    lv_obj_set_style_radius(end_date_btn, 8, 0);
    lv_obj_align(end_date_btn, LV_ALIGN_TOP_MID, 0, 125);
    lv_obj_add_event_cb(end_date_btn, end_date_btn_event_handler, LV_EVENT_ALL, NULL);
    
    end_date_label = lv_label_create(end_date_btn);
    char end_date_text[MAX_MEDICINE_STRING_LENGTH];
    snprintf(end_date_text, sizeof(end_date_text), "End: %02d.%02d.%04d", 
             medicine->treatment_end_date.tm_mday,
             medicine->treatment_end_date.tm_mon + 1, 
             medicine->treatment_end_date.tm_year + 1900);

    lv_label_set_text(end_date_label, end_date_text);
    lv_obj_set_style_text_color(end_date_label, LVGL_BLACK_COLOR, 0);
    lv_obj_center(end_date_label);

    // Special requirements
    special_req_btn = lv_btn_create(scr);
    lv_obj_set_size(special_req_btn, lv_pct(70), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(special_req_btn, LVGL_WHITE_COLOR, 0);
    lv_obj_set_style_radius(special_req_btn, 8, 0);
    lv_obj_align(special_req_btn, LV_ALIGN_TOP_MID, 0, 160);
    lv_obj_add_event_cb(special_req_btn, special_req_btn_event_handler, LV_EVENT_ALL, NULL);
    
    special_req_label = lv_label_create(special_req_btn);
    lv_label_set_long_mode(special_req_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
    lv_label_set_text(special_req_label, medicine->special_requirements);
    lv_obj_set_style_text_color(special_req_label, LVGL_BLACK_COLOR, 0);
    lv_obj_set_width(special_req_label, lv_pct(90)); // Set width for scrolling
    lv_obj_center(special_req_label);
 

    // Back button (blue circle with arrow)
    add_back_btn(scr);

    lv_screen_load(scr);
    ESP_LOGI(TAG, "Add medicine screen loaded");
}

// === Event Handlers ===

static void name_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xE0E0E0), 0);
    }
    else if (code == LV_EVENT_CLICKED)
    {
        ESP_LOGI(TAG, "Name input clicked - opening keyboard");
        // TODO: Open keyboard screen
        // init_lvgl_keyboard_scr(new_medicine.name, sizeof(new_medicine.name), update_name_callback);

        set_section(10);
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, LVGL_WHITE_COLOR, 0);
    }
}

static void frequency_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xE0E0E0), 0);
    }
    else if (code == LV_EVENT_CLICKED)
    {
        set_section(5);
        ESP_LOGI(TAG, "Frequency input clicked - opening frequency selector");
        // TODO: Open frequency selector (roller or number picker)
        // For now, simulate setting frequency to 2
       
        ESP_LOGI(TAG, "Frequency set to: %d", new_medicine.doses_per_day);
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, LVGL_WHITE_COLOR, 0);
    }
}

static void start_date_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xE0E0E0), 0);
    }
    else if (code == LV_EVENT_CLICKED)
    {
        turn_alarm(true);
        ESP_LOGI(TAG, "Start date clicked - opening calendar");
        // TODO: Open calendar screen
        // init_lvgl_calendar_scr(&new_medicine.treatment_start_date, update_start_date_callback);
        
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, LVGL_WHITE_COLOR, 0);
    }
}

static void end_date_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xE0E0E0), 0);
    }
    else if (code == LV_EVENT_CLICKED)
    {
        ESP_LOGI(TAG, "End date clicked - opening calendar");
        // TODO: Open calendar screen
        // init_lvgl_calendar_scr(&new_medicine.treatment_end_date, update_end_date_callback);
        
        // For now, simulate setting date (30 days from start)
       
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, LVGL_WHITE_COLOR, 0);
    }
}


static void special_req_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xE0E0E0), 0);
    }
    else if (code == LV_EVENT_CLICKED)
    {
        ESP_LOGI(TAG, "Special requirements clicked - opening input screen");
        // TODO: Open input screen for special requirements
        // init_lvgl_keyboard_scr(new_medicine.special_requirements, sizeof(new_medicine.special_requirements), update_special_req_callback);
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, LVGL_WHITE_COLOR, 0);
    }
}



// Public callback functions to update fields from other screens
void update_medicine_name_callback(const char *new_name)
{
    if (new_name != NULL && strlen(new_name) > 0)
    {
    
        ESP_LOGI(TAG, "Medicine name updated to: %s", new_medicine.name);
    }
}

void update_frequency_callback(uint16_t doses)
{
    if (doses > 0 && doses <= MAX_MEDICINE_DOSES_NUMBER_PER_DAY)
    {
        
        ESP_LOGI(TAG, "Frequency updated to: %d", doses);
    }
}

void update_start_date_callback(struct tm *date)
{
    if (date != NULL)
    {

        ESP_LOGI(TAG, "Start date updated");
    }
}

void update_end_date_callback(struct tm *date)
{
    if (date != NULL)
    {
       
        ESP_LOGI(TAG, "End date updated");
    }
}