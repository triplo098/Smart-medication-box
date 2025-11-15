#include "lvgl_add_medication_scr.h"
#include "lvgl_medications_list_scr.h"
#include "lvgl_default_scr.h"
// #include "lvgl_keyboard_scr.h"
// #include "lvgl_calendar_scr.h"
// #include "lvgl_time_picker_scr.h"
#include "medicines_managment.h"
#include "display_driver.h"

#include "esp_log.h"
#include <string.h>
#include <stdio.h>

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

// Medicine data
static medicine_t new_medicine;
static bool name_set = false;
static bool frequency_set = false;
static bool start_date_set = false;
static bool end_date_set = false;

// Event handlers declarations
static void name_btn_event_handler(lv_event_t *e);
static void frequency_btn_event_handler(lv_event_t *e);
static void start_date_btn_event_handler(lv_event_t *e);
static void end_date_btn_event_handler(lv_event_t *e);
static void back_btn_event_handler(lv_event_t *e);

// Helper to create a white rounded button
static lv_obj_t *create_input_button(lv_obj_t *parent, const char *text, lv_event_cb_t event_cb, lv_obj_t **label_ref)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 310, 50);
    lv_obj_set_style_bg_color(btn, LVGL_WHITE_COLOR, 0);
    lv_obj_set_style_radius(btn, 25, 0); // Rounded corners
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0x666666), 0); // Gray text for placeholder
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 20, 0);
    
    if (label_ref != NULL)
    {
        *label_ref = label;
    }
    
    if (event_cb != NULL)
    {
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_ALL, NULL);
    }
    
    return btn;
}

void init_lvgl_add_medication_scr()
{   

    ESP_LOGI(TAG, "Initializing add medication screen");

    scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, LCD_H_RES, LCD_V_RES);
    lv_obj_set_style_bg_color(scr, LVGL_BLACK_COLOR, 0);
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);


    // Initialize medicine structure
    memset(&new_medicine, 0, sizeof(medicine_t));
    strcpy(new_medicine.name, "Nazwa");
    new_medicine.doses_per_day = 0;
    name_set = false;
    frequency_set = false;
    start_date_set = false;
    end_date_set = false;

    if (scr == NULL)
    {
        scr = lv_obj_create(NULL);
    }
    else
    {
        lv_obj_clean(scr);
    }

    lv_obj_set_size(scr, 200, 200);
    lv_obj_center(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x3A3A3A), 0); // Dark gray background
    lv_obj_set_style_radius(scr, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_shadow_width(scr, 0, 0);

    // Title bar with blue background
    lv_obj_t *title_bar = lv_obj_create(scr);
    lv_obj_set_size(title_bar, 360, 55);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x5B7FA6), 0); // Blue-gray
    lv_obj_set_style_radius(title_bar, 27, 0); // Rounded top
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_shadow_width(title_bar, 0, 0);
    lv_obj_set_style_pad_all(title_bar, 0, 0);

    lv_obj_t *title = lv_label_create(title_bar);
    lv_label_set_text(title, "Dodawanie leku");
    lv_obj_set_style_text_color(title, LVGL_WHITE_COLOR, 0);
    lv_obj_center(title);

    // Container for input fields
    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, 360, 280);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_row(cont, 10, 0);


    // Back button (blue circle with arrow)
    lv_obj_t *back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 60, 60);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x4A6FA5), 0); // Blue
    lv_obj_set_style_radius(back_btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    lv_obj_add_event_cb(back_btn, back_btn_event_handler, LV_EVENT_ALL, NULL);

    lv_obj_t *back_icon = lv_label_create(back_btn);
    lv_label_set_text(back_icon, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(back_icon, LVGL_WHITE_COLOR, 0);
    lv_obj_set_style_text_font(back_icon, &lv_font_montserrat_16, 0);
    lv_obj_center(back_icon);

    lv_screen_load(scr);
    ESP_LOGI(TAG, "Add medication screen loaded");
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
        ESP_LOGI(TAG, "Frequency input clicked - opening frequency selector");
        // TODO: Open frequency selector (roller or number picker)
        // For now, simulate setting frequency to 2
        new_medicine.doses_per_day = 2;
        frequency_set = true;
        
        char freq_text[32];
        snprintf(freq_text, sizeof(freq_text), "%d razy dziennie", new_medicine.doses_per_day);
        lv_label_set_text(frequency_label, freq_text);
        lv_obj_set_style_text_color(frequency_label, LVGL_BLACK_COLOR, 0);
        
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
        ESP_LOGI(TAG, "Start date clicked - opening calendar");
        // TODO: Open calendar screen
        // init_lvgl_calendar_scr(&new_medicine.treatment_start_date, update_start_date_callback);
        
        // For now, simulate setting date
        time_t now = time(NULL);
        localtime_r(&now, &new_medicine.treatment_start_date);
        start_date_set = true;
        
        char date_text[32];
        strftime(date_text, sizeof(date_text), "%d.%m.%Y", &new_medicine.treatment_start_date);
        lv_label_set_text(start_date_label, date_text);
        lv_obj_set_style_text_color(start_date_label, LVGL_BLACK_COLOR, 0);
        
        ESP_LOGI(TAG, "Start date set to: %s", date_text);
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
        if (start_date_set)
        {
            new_medicine.treatment_end_date = new_medicine.treatment_start_date;
            new_medicine.treatment_end_date.tm_mday += 30;
            mktime(&new_medicine.treatment_end_date); // Normalize
            end_date_set = true;
            
            char date_text[32];
            strftime(date_text, sizeof(date_text), "%d.%m.%Y", &new_medicine.treatment_end_date);
            lv_label_set_text(end_date_label, date_text);
            lv_obj_set_style_text_color(end_date_label, LVGL_BLACK_COLOR, 0);
            
            ESP_LOGI(TAG, "End date set to: %s", date_text);
        }
        else
        {
            ESP_LOGW(TAG, "Please set start date first!");
        }
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, LVGL_WHITE_COLOR, 0);
    }
}

static void back_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x3A5A8A), 0);
    }
    else if (code == LV_EVENT_CLICKED)
    {
        ESP_LOGI(TAG, "Back button clicked");
        
        // Check if we should save
        if (name_set && frequency_set && start_date_set && end_date_set)
        {
            ESP_LOGI(TAG, "Saving medicine: %s", new_medicine.name);
            add_medicine(&new_medicine);
            // save_medicines_to_storage();
        }
        else
        {
            ESP_LOGI(TAG, "Not all fields filled, returning without saving");
        }
        
        // Go back to medications list
        init_lvgl_medications_list_scr();
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x4A6FA5), 0);
    }
}

// Public callback functions to update fields from other screens

void update_medicine_name_callback(const char *new_name)
{
    if (new_name != NULL && strlen(new_name) > 0)
    {
        strncpy(new_medicine.name, new_name, MAX_MEDICINE_STRING_LENGTH - 1);
        new_medicine.name[MAX_MEDICINE_STRING_LENGTH - 1] = '\0';
        name_set = true;
        
        lv_label_set_text(name_label, new_medicine.name);
        lv_obj_set_style_text_color(name_label, LVGL_BLACK_COLOR, 0);
        
        ESP_LOGI(TAG, "Medicine name updated to: %s", new_medicine.name);
    }
}

void update_frequency_callback(uint16_t doses)
{
    if (doses > 0 && doses <= MAX_MEDICINE_DOSES_NUMBER_PER_DAY)
    {
        new_medicine.doses_per_day = doses;
        frequency_set = true;
        
        char freq_text[32];
        snprintf(freq_text, sizeof(freq_text), "%d razy dziennie", doses);
        lv_label_set_text(frequency_label, freq_text);
        lv_obj_set_style_text_color(frequency_label, LVGL_BLACK_COLOR, 0);
        
        ESP_LOGI(TAG, "Frequency updated to: %d", doses);
    }
}

void update_start_date_callback(struct tm *date)
{
    if (date != NULL)
    {
        new_medicine.treatment_start_date = *date;
        start_date_set = true;
        
        char date_text[32];
        strftime(date_text, sizeof(date_text), "%d.%m.%Y", date);
        lv_label_set_text(start_date_label, date_text);
        lv_obj_set_style_text_color(start_date_label, LVGL_BLACK_COLOR, 0);
        
        ESP_LOGI(TAG, "Start date updated");
    }
}

void update_end_date_callback(struct tm *date)
{
    if (date != NULL)
    {
        new_medicine.treatment_end_date = *date;
        end_date_set = true;
        
        char date_text[32];
        strftime(date_text, sizeof(date_text), "%d.%m.%Y", date);
        lv_label_set_text(end_date_label, date_text);
        lv_obj_set_style_text_color(end_date_label, LVGL_BLACK_COLOR, 0);
        
        ESP_LOGI(TAG, "End date updated");
    }
}