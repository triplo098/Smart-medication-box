#include "medicines_managment.h"
#include "pcf8563.h"
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include "esp_log.h"
#include "alarm_helpers.h"
#include "sections_controller.h"

#include "lvgl_alarm_scr.h"

medicine_t **medicines_list;

extern i2c_dev_t rtc_dev;

static const char *TAG = "MED_MGMT";
extern struct tm current_time;


section sections[NUMEBR_OF_SECTIONS] = {0};



void get_next_medicine_time_from_all_medicines(time_mh_t *next_time)
{
    ESP_LOGI(TAG, "Calculating next medicine time...");
    ESP_LOGI(TAG, "Current time: %02d:%02d",
             current_time.tm_hour, current_time.tm_min);
    
    
    long long int nearest_ts = LONG_MAX;


    for (int i = 0; i < MAX_MEDICINES_TOTAL; i++)
    {
        // ESP_LOGI(TAG, "Medicine index:%d", i);

        if (medicines_list[i] == NULL)
            continue;

        ESP_LOGI(TAG, "Checking medicine: %s", medicines_list[i]->name);

        // Check if medicine is within treatment dates
        if((mktime(&current_time) < mktime(&medicines_list[i]->treatment_start_date)) ||
           (mktime(&current_time) > mktime(&medicines_list[i]->treatment_end_date)))
        {
            ESP_LOGI(TAG, "  Medicine %s is out of treatment date range.", medicines_list[i]->name);
            continue;
        }
        

        for (int j = 0; j < medicines_list[i]->doses_per_day; j++)
        {
            time_mh_t mh = medicines_list[i]->dose_times[j];

            ESP_LOGI(TAG, "  Dose time: %02d:%02d", mh.hour, mh.minute);
            
            // Create dose time for TODAY
            struct tm dose_tm = current_time;
            dose_tm.tm_hour = mh.hour;
            dose_tm.tm_min = mh.minute;
            dose_tm.tm_sec = 59;

            current_time.tm_sec = 0; // Current time seconds set to 0 for comparison
            
            time_t dose_ts = mktime(&dose_tm);
            time_t now_ts = mktime(&current_time);
            if (dose_ts <= now_ts)
            {
                ESP_LOGI(TAG, "    Dose time already passed today.");
                // Dose time already passed today, consider next day
                dose_tm.tm_mday += 1;
                dose_ts = mktime(&dose_tm);
            }

            if (dose_ts <= nearest_ts)
            {
                nearest_ts = dose_ts;
                ESP_LOGI(TAG, "    Nearest dose time updated to: %02d:%02d",
                         mh.hour, mh.minute);
            }
        }
    }

    if (nearest_ts == LONG_MAX)
    {
        ESP_LOGE(TAG, "No medicine times found!");
        next_time->hour = 0;
        next_time->minute = 0;
        return;
    }

    struct tm *result = localtime(&nearest_ts);
    next_time->hour = result->tm_hour;
    next_time->minute = result->tm_min;

    ESP_LOGI(TAG, "Next dose at: %02d:%02d",
             next_time->hour, next_time->minute);
}

void medicine_list_init()
{
    medicines_list = heap_caps_calloc(MAX_MEDICINES_TOTAL, sizeof(medicine_t *), MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "Medicine list initialized with %d slots.", MAX_MEDICINES_TOTAL);
}


void add_medicine(medicine_t *medicine)
{
    ESP_LOGI(TAG, "Adding medicine: %s", medicine->name);
    for (int i = 0; i < MAX_MEDICINES_TOTAL; i++)
    {
        if (medicines_list[i] == NULL)
        {
            ESP_LOGI(TAG, "Found empty slot at index %d", i);
            medicines_list[i] = medicine;
            break;
        }
    }

    log_medicine_info(medicine);

}

void remove_medicine(const char *name)
{
    ESP_LOGI(TAG, "Removing medicine: %s", name);
    for (int i = 0; i < MAX_MEDICINES_TOTAL; i++)
    {
        if (strcmp(medicines_list[i]->name, name) == 0)
        {
            ESP_LOGI(TAG, "Medicine found at index %d, removing...", i);

            if (medicines_list[i] != NULL)
            {
                heap_caps_free(medicines_list[i]);
                medicines_list[i] = NULL;
            }
            return;
        }
    }
    ESP_LOGI(TAG, "Medicine not found: %s", name);
}


medicine_t* get_medicine_from_name(const char *name) {
    for (int i = 0; i < MAX_MEDICINES_TOTAL; i++)
    {
        if (medicines_list[i] != NULL && strcmp(medicines_list[i]->name, name) == 0)
        {
            return medicines_list[i];
        }
    }
    return NULL;
}

void check_for_alarm_task(void *arg)
{
    while (1)
    {

        ESP_LOGI(TAG, "Alarm check");
        
        time_mh_t next_time;
        get_next_medicine_time_from_all_medicines(&next_time);

        bool valid;
        ESP_ERROR_CHECK(pcf8563_get_time(&rtc_dev, &current_time, &valid));

        if (!valid)
        {
            ESP_LOGE(TAG, "RTC time invalid!");
            return;
        }

        ESP_LOGI(TAG, "Current RTC time: %02d:%02d",
                 current_time.tm_hour, current_time.tm_min);

        ESP_LOGI(TAG, "Next medicine time: %02d:%02d",
                 next_time.hour, next_time.minute);

        if (current_time.tm_hour == next_time.hour && current_time.tm_min == next_time.minute)
        {
            ESP_LOGI(TAG, "Alarm time reached! Triggering alarm...");
            turn_alarm(true);
            init_lvgl_alarm_scr();
        }

        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}


void log_medicine_info(medicine_t* medicine)
{   

    ESP_LOGI(TAG, "---- Medicine Info ----");

    ESP_LOGI(TAG, "Medicine Name: %s", medicine->name);
    ESP_LOGI(TAG, "Doses per day: %d", medicine->doses_per_day);
    for (int i = 0; i < medicine->doses_per_day; i++)
    {
        ESP_LOGI(TAG, "  Dose %d time: %02d:%02d", i + 1,
                 medicine->dose_times[i].hour,
                 medicine->dose_times[i].minute);
    }

    ESP_LOGI(TAG, "Special Requirements: %s", medicine->special_requirements);
    ESP_LOGI(TAG, "Treatment Start Date: %04d-%02d-%02d",
             medicine->treatment_start_date.tm_year + 1900,
             medicine->treatment_start_date.tm_mon + 1,
             medicine->treatment_start_date.tm_mday);
    ESP_LOGI(TAG, "Treatment End Date: %04d-%02d-%02d",
             medicine->treatment_end_date.tm_year + 1900,
             medicine->treatment_end_date.tm_mon + 1,
             medicine->treatment_end_date.tm_mday);

    ESP_LOGI(TAG, "-----------------------");
}


void get_medicines_list(medicine_t*** medicines, size_t* count){
    size_t med_count = 0;
    for (int i = 0; i < MAX_MEDICINES_TOTAL; i++)
    {
        if (medicines_list[i] != NULL)
        {
            med_count++;
        }
    }

    *medicines = medicines_list;
    *count = med_count;
}

void get_next_medicine_time_from_medicine(medicine_t* medicine, struct tm* next_time)
{
    time_t now_ts = time(NULL);
    localtime_r(&now_ts, &current_time);

    ESP_LOGI(TAG, "Calculating next time for medicine: %s", medicine->name);

    // Convert treatment start/end to timestamps
    time_t start_ts = mktime(&medicine->treatment_start_date);
    time_t end_ts   = mktime(&medicine->treatment_end_date);

    if (now_ts > end_ts)
    {
        ESP_LOGW(TAG, "Treatment already ended");
        *next_time = medicine->treatment_end_date;
        return;
    }

    // Start iterating from today or treatment start — whichever is later
    time_t day_ts = (now_ts > start_ts) ? now_ts : start_ts;

    // Normalize to midnight of current day
    struct tm day_tm;
    localtime_r(&day_ts, &day_tm);
    day_tm.tm_hour = 0;
    day_tm.tm_min = 0;
    day_tm.tm_sec = 0;
    day_ts = mktime(&day_tm);

    struct tm best_time = {0};
    bool found = false;

    while (day_ts <= end_ts)
    {
        localtime_r(&day_ts, &day_tm);

        for (int i = 0; i < medicine->doses_per_day; i++)
        {
            struct tm dose_tm = day_tm;
            dose_tm.tm_hour  = medicine->dose_times[i].hour;
            dose_tm.tm_min   = medicine->dose_times[i].minute;
            dose_tm.tm_sec   = 0;

            time_t dose_ts = mktime(&dose_tm);

            // Must be in the future
            if (dose_ts >= now_ts)
            {
                if (!found || difftime(dose_ts, mktime(&best_time)) < 0)
                {
                    best_time = dose_tm;
                    found = true;
                }
            }
        }

        // Move to next day
        day_ts += 24 * 3600;
    }

    if (found)
    {
        *next_time = best_time;
        ESP_LOGI(TAG, "Next dose: %04d-%02d-%02d %02d:%02d",
                 next_time->tm_year + 1900,
                 next_time->tm_mon + 1,
                 next_time->tm_mday,
                 next_time->tm_hour,
                 next_time->tm_min);
    }
    else
    {
        ESP_LOGW(TAG, "No future dose found");
        *next_time = medicine->treatment_end_date;
    }
}

void get_all_medicine_time(struct tm * times_array, size_t *count, medicine_t *medicine)
{
    if (!times_array || !count || !medicine) {
        if (count) *count = 0;
        return;
    }
    
    *count = 0;
    
    // Normalize start and end dates to midnight
    // struct tm start = medicine->treatment_start_date;
    // start.tm_hour = 0;
    // start.tm_min = 0;
    // start.tm_sec = 0;
    // start.tm_isdst = -1;


    struct tm start = current_time;
    
    
    struct tm end = medicine->treatment_end_date;
    end.tm_hour = 23;
    end.tm_min = 59;
    end.tm_sec = 59;
    end.tm_isdst = -1;
    
    // Convert to time_t for easier comparison
    time_t start_time = mktime(&start);
    time_t end_time = mktime(&end);
    
    if (start_time == -1 || end_time == -1 || start_time > end_time) {
        return;
    }
    

    // Iterate through each day in the treatment period
    struct tm current_day = start;
    time_t current_time = start_time;
    
    while (current_time <= end_time) {
        // For each dose time in the day
        for (uint16_t i = 0; i < medicine->doses_per_day && i < MAX_MEDICINE_DOSES_NUMBER_PER_DAY; i++) {
            // Create a time struct for this specific dose
            struct tm dose_time = current_day;
            dose_time.tm_hour = medicine->dose_times[i].hour;
            dose_time.tm_min = medicine->dose_times[i].minute;
            dose_time.tm_sec = 0;
            dose_time.tm_isdst = -1;
            
            // Verify this dose time falls within the treatment window
            time_t dose_timestamp = mktime(&dose_time);
            if (dose_timestamp >= start_time && dose_timestamp <= end_time) {
                times_array[*count] = dose_time;
                (*count)++;
            }
        }
        
        // Move to next day
        current_day.tm_mday++;
        current_day.tm_isdst = -1;
        current_time = mktime(&current_day);
        
        if (current_time == -1) {
            break; // Error in date calculation
        }
    }
}


void set_medicine_times_from_frequency(medicine_t* medicine){



    switch(medicine->doses_per_day) {

        case 0:
            // No doses
            break;
        case 1:
            medicine->dose_times[0].hour = 8;
            medicine->dose_times[0].minute = 0;
            break;
        case 2:
            medicine->dose_times[0].hour = 8;
            medicine->dose_times[0].minute = 0;
            medicine->dose_times[1].hour = 20;
            medicine->dose_times[1].minute = 0;
            break;

        case 3:

            medicine->dose_times[0].hour = 8;
            medicine->dose_times[0].minute = 0;
            medicine->dose_times[1].hour = 14;
            medicine->dose_times[1].minute = 0;
            medicine->dose_times[2].hour = 20;
            medicine->dose_times[2].minute = 0;
            break;

        case 4:

            medicine->dose_times[0].hour = 6;
            medicine->dose_times[0].minute = 0;
            medicine->dose_times[1].hour = 12;
            medicine->dose_times[1].minute = 0;
            medicine->dose_times[2].hour = 18;
            medicine->dose_times[2].minute = 0;
            medicine->dose_times[3].hour = 22;
            medicine->dose_times[3].minute = 0;
            break;

        case 5:
            medicine->dose_times[0].hour = 6;
            medicine->dose_times[0].minute = 0;
            medicine->dose_times[1].hour = 10;
            medicine->dose_times[1].minute = 0;
            medicine->dose_times[2].hour = 14;
            medicine->dose_times[2].minute = 0;
            medicine->dose_times[3].hour = 18;
            medicine->dose_times[3].minute = 0;
            medicine->dose_times[4].hour = 22;
            medicine->dose_times[4].minute = 0;
            break;
        case 6:
            medicine->dose_times[0].hour = 6;
            medicine->dose_times[0].minute = 0;
            medicine->dose_times[1].hour = 10;
            medicine->dose_times[1].minute = 0;
            medicine->dose_times[2].hour = 14;
            medicine->dose_times[2].minute = 0;
            medicine->dose_times[3].hour = 18;
            medicine->dose_times[3].minute = 0;
            medicine->dose_times[4].hour = 22;
            medicine->dose_times[4].minute = 0;
            medicine->dose_times[5].hour = 2;
            medicine->dose_times[5].minute = 0;
            break;
        default:
            ESP_LOGW(TAG, "Unsupported doses per day: %d", medicine->doses_per_day);
            break;
    }
}

