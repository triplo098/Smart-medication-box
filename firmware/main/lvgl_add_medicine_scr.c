#include "lvgl_add_medicine_scr.h"
#include "lvgl_medicines_list_scr.h"
#include "lvgl_default_scr.h"
#include "lvgl_refill_medicine_scr.h"

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
extern i2c_dev_t rtc_dev;

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

// Helper functions
static void lvgl_keyboard(lv_obj_t *ta);

static void add_back_btn(lv_obj_t *scr);

// Medicine data
static medicine_t *local_medicine;
static bool is_new_medicine = false;


// Event handlers declarations
static void name_btn_event_handler(lv_event_t *e);
static void frequency_btn_event_handler(lv_event_t *e);
static void start_date_btn_event_handler(lv_event_t *e);
static void end_date_btn_event_handler(lv_event_t *e);
static void back_btn_event_handler(lv_event_t *e);
static void special_req_btn_event_handler(lv_event_t *e);
static void save_btn_event_handler(lv_event_t *e);


void init_lvgl_add_medicine_scr(medicine_t *medicine)
{

    ESP_LOGI(TAG, "Initializing add medicine screen");

    scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, LCD_H_RES, LCD_V_RES);
    lv_obj_set_style_bg_color(scr, LVGL_BLACK_COLOR, 0);
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);

    // Initialize medicine structure
    if (medicine == NULL)
    {
        medicine = heap_caps_malloc(sizeof(medicine_t), MALLOC_CAP_8BIT);
        strcpy(medicine->name, "Name");
        medicine->doses_per_day = 1;
        medicine->dose_times[0].hour = 8;
        medicine->dose_times[0].minute = 0;
        medicine->treatment_start_date = current_time;
        medicine->treatment_end_date = current_time;

        medicine->treatment_end_date.tm_mday += 1 % 31;
        mktime(&medicine->treatment_end_date); // Normalize the date
        strcpy(medicine->special_requirements, "Special requirements: none. Tap to edit.");
        is_new_medicine = true;
    }

    ESP_LOGI(TAG, "Current time: %04d-%02d-%02d %02d:%02d:%02d",
             current_time.tm_year + 1900,
             current_time.tm_mon + 1,
             current_time.tm_mday,
             current_time.tm_hour,
             current_time.tm_min,
             current_time.tm_sec);

    local_medicine = medicine;

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
    lv_label_set_text(special_req_label, local_medicine->special_requirements);
    lv_obj_set_style_text_color(special_req_label, LVGL_BLACK_COLOR, 0);
    lv_obj_set_width(special_req_label, lv_pct(90)); // Set width for scrolling
    lv_obj_center(special_req_label);

    // Back button (blue circle with arrow)
    add_back_btn(scr);

    
    // Add save button to the right of back button
    lv_obj_t *save_btn = lv_btn_create(scr);
    lv_obj_set_size(save_btn, 30, 30);  // Same size as back button
    lv_obj_set_style_bg_color(save_btn, LVGL_DARK_GREEN_COLOR, 0);
    lv_obj_set_style_radius(save_btn, 8, 0);
    lv_obj_align(save_btn, LV_ALIGN_BOTTOM_MID, 25, -15);  // Aligned to BOTTOM_RIGHT
    
    lv_obj_t *save_label = lv_label_create(save_btn);
    lv_label_set_text(save_label, LV_SYMBOL_OK);
    lv_obj_set_style_text_color(save_label, LVGL_WHITE_COLOR, 0);
    lv_obj_center(save_label);
    lv_obj_add_event_cb(save_btn, save_btn_event_handler, LV_EVENT_ALL, NULL);


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

        // Create a textarea for input
        lv_obj_t *ta = lv_textarea_create(scr);
        lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 20);
        lv_obj_set_size(ta, lv_pct(70), 40);
        lv_textarea_set_text(ta, local_medicine->name);
        lv_obj_add_state(ta, LV_STATE_FOCUSED);

        // Open keyboard with the textarea
        lvgl_keyboard(ta);
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, LVGL_WHITE_COLOR, 0);
    }
}

static lv_obj_t *frequency_dropdown_obj = NULL;
static lv_obj_t *frequency_close_btn_obj = NULL;

static void frequency_btn_matrix_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_current_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        uint32_t id = lv_buttonmatrix_get_selected_button(obj);
        const char *txt = lv_buttonmatrix_get_button_text(obj, id);
        
        if (txt) {
            int doses = atoi(txt);
            if (doses >= 1 && doses <= 6) {
                local_medicine->doses_per_day = doses;
                
                // Update the frequency label
                char freq_text[32];
                snprintf(freq_text, sizeof(freq_text), "%d daily", doses);
                lv_label_set_text(frequency_label, freq_text);
                
                ESP_LOGI(TAG, "Frequency set to: %d", doses);
                
                // Close the dropdown
                if (frequency_dropdown_obj) {
                    lv_obj_del(frequency_dropdown_obj);
                    frequency_dropdown_obj = NULL;
                }
                if (frequency_close_btn_obj) {
                    lv_obj_del(frequency_close_btn_obj);
                    frequency_close_btn_obj = NULL;
                }
            }
        }
    }
}

static void frequency_close_btn_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Frequency close button clicked");
        
        // Delete frequency dropdown objects
        if (frequency_dropdown_obj) {
            lv_obj_del(frequency_dropdown_obj);
            frequency_dropdown_obj = NULL;
        }
        if (frequency_close_btn_obj) {
            lv_obj_del(frequency_close_btn_obj);
            frequency_close_btn_obj = NULL;
        }
    }
}

static void show_frequency_selector(void)
{
    // Clean up any existing frequency selector first
    if (frequency_dropdown_obj) {
        lv_obj_del(frequency_dropdown_obj);
        frequency_dropdown_obj = NULL;
    }
    if (frequency_close_btn_obj) {
        lv_obj_del(frequency_close_btn_obj);
        frequency_close_btn_obj = NULL;
    }

    // Create container
    frequency_dropdown_obj = lv_obj_create(lv_screen_active());
    lv_obj_set_size(frequency_dropdown_obj, lv_pct(85), lv_pct(60));
    lv_obj_center(frequency_dropdown_obj);
    lv_obj_set_style_bg_color(frequency_dropdown_obj, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(frequency_dropdown_obj, 15, 0);
    lv_obj_set_style_border_width(frequency_dropdown_obj, 0, 0);

    // Create title label
    lv_obj_t *title_label = lv_label_create(frequency_dropdown_obj);
    lv_label_set_text(title_label, "Frequency");
    lv_obj_set_style_text_color(title_label, LVGL_WHITE_COLOR, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10);

    // Create button matrix for frequency selection
    static const char * btnm_map[] = {"1x", "2x", "3x", "\n",
                                       "4x", "5x", "6x", ""};

    lv_obj_t *btnm = lv_buttonmatrix_create(frequency_dropdown_obj);
    lv_buttonmatrix_set_map(btnm, btnm_map);
    lv_obj_set_size(btnm, lv_pct(85), lv_pct(70));
    lv_obj_align(btnm, LV_ALIGN_CENTER, 0, 5);
    
    // Style the button matrix
    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x333333), 0);
    lv_obj_set_style_border_width(btnm, 0, 0);
    
    // Style buttons
    lv_obj_set_style_bg_color(btnm, lv_color_hex(0xEEEEEE), LV_PART_ITEMS);
    lv_obj_set_style_text_color(btnm, lv_color_hex(0x000000), LV_PART_ITEMS);
    lv_obj_set_style_radius(btnm, 8, LV_PART_ITEMS);
    
    // Style selected button
    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x4A90E2), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(btnm, lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_CHECKED);
    
    // Highlight current selection
    if (local_medicine->doses_per_day >= 1 && local_medicine->doses_per_day <= 6) {
        lv_buttonmatrix_set_button_ctrl(btnm, local_medicine->doses_per_day - 1, LV_BUTTONMATRIX_CTRL_CHECKED);
    }

    lv_obj_add_event_cb(btnm, frequency_btn_matrix_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    // Add close button (back arrow)
    frequency_close_btn_obj = lv_btn_create(frequency_dropdown_obj);
    lv_obj_set_size(frequency_close_btn_obj, 30, 30);
    lv_obj_align(frequency_close_btn_obj, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_color(frequency_close_btn_obj, LVGL_DARK_BLUE_COLOR, 0);
    lv_obj_set_style_radius(frequency_close_btn_obj, 8, 0);
    
    lv_obj_t *arrow_label = lv_label_create(frequency_close_btn_obj);
    lv_label_set_text(arrow_label, LV_SYMBOL_LEFT);
    lv_obj_center(arrow_label);
    
    
    lv_obj_add_event_cb(frequency_close_btn_obj, frequency_close_btn_handler, LV_EVENT_CLICKED, NULL);
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
        ESP_LOGI(TAG, "Frequency input clicked - opening frequency selector");
        show_frequency_selector();
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, LVGL_WHITE_COLOR, 0);
    }
}




static lv_obj_t *calendar_obj = NULL;
static lv_obj_t *calendar_close_btn_obj = NULL;
static bool is_start_date_selection = true;

static lv_obj_t *calendar_header = NULL;

static void calendar_header_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_current_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t date;
        if(lv_calendar_get_pressed_date(calendar_obj, &date)) {
            lv_calendar_set_showed_date(calendar_obj, date.year, date.month);
        }
    }
}

static void calendar_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_current_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t date;
        if (lv_calendar_get_pressed_date(obj, &date)) {
            ESP_LOGI(TAG, "Clicked date: %02d.%02d.%04d", date.day, date.month, date.year);
            
            // Update the appropriate date in medicine structure
            struct tm *target_date;
            lv_obj_t *target_label;
            
            if (is_start_date_selection) {
                target_date = &local_medicine->treatment_start_date;
                target_label = start_date_label;
            } else {
                target_date = &local_medicine->treatment_end_date;
                target_label = end_date_label;
            }
            
            // Update the date
            target_date->tm_year = date.year - 1900;
            target_date->tm_mon = date.month - 1;
            target_date->tm_mday = date.day;
            mktime(target_date); // Normalize the date
            
            // Update the label
            char date_text[MAX_MEDICINE_STRING_LENGTH];
            snprintf(date_text, sizeof(date_text), "%s: %02d.%02d.%04d",
                     is_start_date_selection ? "Start" : "End",
                     target_date->tm_mday,
                     target_date->tm_mon + 1,
                     target_date->tm_year + 1900);
            lv_label_set_text(target_label, date_text);
            
            ESP_LOGI(TAG, "Date updated: %s", date_text);
            
            // Close calendar
            if (calendar_header) {
                lv_obj_del(calendar_header);
                calendar_header = NULL;
            }
            if (calendar_obj) {
                lv_obj_del(calendar_obj);
                calendar_obj = NULL;
            }
            if (calendar_close_btn_obj) {
                lv_obj_del(calendar_close_btn_obj);
                calendar_close_btn_obj = NULL;
            }
        }
    }
}

static void calendar_close_btn_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Calendar close button clicked");
        
        // Delete calendar objects
        if (calendar_header) {
            lv_obj_del(calendar_header);
            calendar_header = NULL;
        }
        if (calendar_obj) {
            lv_obj_del(calendar_obj);
            calendar_obj = NULL;
        }
        if (calendar_close_btn_obj) {
            lv_obj_del(calendar_close_btn_obj);
            calendar_close_btn_obj = NULL;
        }
    }
}

static void show_calendar(bool is_start_date)
{
    // Clean up any existing calendar first
    if (calendar_header) {
        lv_obj_del(calendar_header);
        calendar_header = NULL;
    }
    if (calendar_obj) {
        lv_obj_del(calendar_obj);
        calendar_obj = NULL;
    }
    if (calendar_close_btn_obj) {
        lv_obj_del(calendar_close_btn_obj);
        calendar_close_btn_obj = NULL;
    }

    is_start_date_selection = is_start_date;
    
    // Create calendar
    calendar_obj = lv_calendar_create(lv_screen_active());
    lv_obj_set_size(calendar_obj, lv_pct(90), lv_pct(70));
    lv_obj_align(calendar_obj, LV_ALIGN_CENTER, 0, 10);
    
    // Set style
    lv_obj_set_style_bg_color(calendar_obj, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_color(calendar_obj, lv_color_hex(0xFFFFFF), 0);
    
    // Create header with month and year dropdowns
    calendar_header = lv_calendar_header_dropdown_create(calendar_obj);
    lv_obj_set_style_bg_color(calendar_header, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_color(calendar_header, lv_color_hex(0xFFFFFF), 0);

    
    // Set today's date
    lv_calendar_set_today_date(calendar_obj, 
                                current_time.tm_year + 1900, 
                                current_time.tm_mon + 1, 
                                current_time.tm_mday);
    
    // Get the date to show
    struct tm *current_date = is_start_date ? 
        &local_medicine->treatment_start_date : 
        &local_medicine->treatment_end_date;
    
    // Show the month of selected date
    lv_calendar_set_showed_date(calendar_obj, 
                                 current_date->tm_year + 1900, 
                                 current_date->tm_mon + 1);
    
    // Highlight selected date
    lv_calendar_date_t highlighted[] = {
        {
            .year = current_date->tm_year + 1900,
            .month = current_date->tm_mon + 1,
            .day = current_date->tm_mday
        }
    };
    lv_calendar_set_highlighted_dates(calendar_obj, highlighted, 1);
    
    // Add event handler
    lv_obj_add_event_cb(calendar_obj, calendar_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Add close button
    calendar_close_btn_obj = lv_btn_create(lv_screen_active());
    lv_obj_set_size(calendar_close_btn_obj, 50, 50);
    lv_obj_align(calendar_close_btn_obj, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(calendar_close_btn_obj, lv_color_hex(0x555555), 0);
    lv_obj_set_style_radius(calendar_close_btn_obj, 25, 0);
    
    lv_obj_t *close_label = lv_label_create(calendar_close_btn_obj);
    lv_label_set_text(close_label, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(close_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(close_label);
    
    lv_obj_add_event_cb(calendar_close_btn_obj, calendar_close_btn_handler, LV_EVENT_CLICKED, NULL);
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
        ESP_LOGI(TAG, "Start date clicked - opening calendar");
        show_calendar(true);
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
        show_calendar(false);
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

static void save_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1B5E20), 0); // Darker green on press
    }
    else if (code == LV_EVENT_CLICKED)
    {
        ESP_LOGI(TAG, "Save button clicked - saving medicine");
        
        if(is_new_medicine)
        {
            add_medicine(local_medicine);
            is_new_medicine = false;
            //Starting refill procedure
            init_refill_medicine_scr(local_medicine);
            

        }
        else
        {
            ESP_LOGI(TAG, "Updating existing medicine: %s", local_medicine->name);
            init_lvgl_medicines_list_scr(NULL);
        }

    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, LVGL_DARK_GREEN_COLOR, 0);
    }
}


// Public callback functions to update fields from other screens
void update_medicine_name_callback(const char *new_name)
{
    if (new_name != NULL && strlen(new_name) > 0)
    {

        ESP_LOGI(TAG, "Medicine name updated to: %s", local_medicine->name);
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

static void back_btn_event_handler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_PRESSED || code == LV_EVENT_CLICKED || code == LV_EVENT_SHORT_CLICKED)
    {
        // Code to handle button press
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0D47A1), 0); // Darker blue on press

        init_lvgl_medicines_list_scr(NULL);
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1976D2), 0); // Original color on release
    }
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
    lv_obj_add_event_cb(back_btn, back_btn_event_handler, LV_EVENT_ALL, NULL);
}

// KEYBAORD
static void keyboard_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_target(e);

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
    {
        lv_obj_t *ta = lv_keyboard_get_textarea(kb);

        if (code == LV_EVENT_READY)
        {
            // OK button pressed - save the text
            const char *text = lv_textarea_get_text(ta);
            strncpy(local_medicine->name, text, sizeof(local_medicine->name) - 1);
            local_medicine->name[sizeof(local_medicine->name) - 1] = '\0';

            // Update the button label
            lv_label_set_text(name_label, local_medicine->name);

            ESP_LOGI(TAG, "Medicine name updated: %s", local_medicine->name);
        }

        // Clean up
        lv_obj_del(ta);
        lv_obj_del(kb);
    }
}

static void lvgl_keyboard(lv_obj_t *ta)
{
    // Simplified keyboard for round display - 8 keys per row
    static const char *kb_map[] = {
        "A", "B", "C", "D", "E", "F", "G", "H", "\n",
        "I", "J", "K", "L", "M", "N", "O", "P", "\n",
        "Q", "R", "S", "T", "U", "V", "W", "X", "\n",
        "Y", "Z", " ", LV_SYMBOL_BACKSPACE, LV_SYMBOL_OK, "\n",
        NULL};

    static const lv_buttonmatrix_ctrl_t kb_ctrl[] = {
        // Row 1 - 8 equal keys
        LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3,
        LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3,
        LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3,

        // Row 2 - 8 equal keys
        LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3,
        LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3,
        LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3,

        // Row 3 - 8 equal keys
        LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3,
        LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3,
        LV_BUTTONMATRIX_CTRL_WIDTH_3, LV_BUTTONMATRIX_CTRL_WIDTH_3,

        // Row 4 - Y, Z, Space, Backspace, OK (more squeezed)
        LV_BUTTONMATRIX_CTRL_WIDTH_2, LV_BUTTONMATRIX_CTRL_WIDTH_2,
        LV_BUTTONMATRIX_CTRL_WIDTH_4,
        LV_BUTTONMATRIX_CTRL_WIDTH_5, LV_BUTTONMATRIX_CTRL_WIDTH_5};

    lv_obj_t *kb = lv_keyboard_create(lv_screen_active());
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_USER_1);

    // Stretch keyboard to 3/4 of vertical space
    lv_obj_set_size(kb, lv_pct(95), lv_pct(75));
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Add event handler for OK/Cancel
    lv_obj_add_event_cb(kb, keyboard_event_handler, LV_EVENT_ALL, NULL);

    lv_keyboard_set_textarea(kb, ta);
}

static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_current_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        lv_calendar_date_t date;
        if (lv_calendar_get_pressed_date(obj, &date))
        {
            LV_LOG_USER("Clicked date: %02d.%02d.%d", date.day, date.month, date.year);
        }
    }
}
