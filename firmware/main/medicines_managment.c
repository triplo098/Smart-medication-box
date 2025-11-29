#include "medicines_managment.h"
#include "pcf8563.h"
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include "esp_log.h"
#include "alarm_helpers.h"

medicine_t **medicines_list;

extern i2c_dev_t rtc_dev;

static const char *TAG = "MED_MGMT";

void get_next_medicine_time(time_mh_t *next_time)
{
    ESP_LOGI(TAG, "Calculating next medicine time...");

    struct tm time_now;
    bool valid;
    ESP_ERROR_CHECK(pcf8563_get_time(&rtc_dev, &time_now, &valid));

    if (!valid)
    {
        ESP_LOGE(TAG, "RTC time invalid!");
        next_time->hour = 0;
        next_time->minute = 0;
        return;
    }

    long long int nearest_ts = LONG_MAX;


    for (int i = 0; i < MAX_MEDICINES_TOTAL; i++)
    {
        // ESP_LOGI(TAG, "Medicine index:%d", i);

        if (medicines_list[i] == NULL)
            continue;

        ESP_LOGI(TAG, "Checking medicine: %s", medicines_list[i]->name);

        // Check if medicine is within treatment dates
        if((mktime(&time_now) < mktime(&medicines_list[i]->treatment_start_date)) ||
           (mktime(&time_now) > mktime(&medicines_list[i]->treatment_end_date)))
        {
            ESP_LOGI(TAG, "  Medicine %s is out of treatment date range.", medicines_list[i]->name);
            continue;
        }
        

        for (int j = 0; j < medicines_list[i]->doses_per_day; j++)
        {
            time_mh_t mh = medicines_list[i]->dose_times[j];

            ESP_LOGI(TAG, "  Dose time: %02d:%02d", mh.hour, mh.minute);
            
            // Create dose time for TODAY
            struct tm dose_tm = time_now;
            dose_tm.tm_hour = mh.hour;
            dose_tm.tm_min = mh.minute;
            dose_tm.tm_sec = 59;

            time_now.tm_sec = 0; // Current time seconds set to 0 for comparison
            
            time_t dose_ts = mktime(&dose_tm);
            time_t now_ts = mktime(&time_now);
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
        get_next_medicine_time(&next_time);

        struct tm time_now;
        bool valid;
        ESP_ERROR_CHECK(pcf8563_get_time(&rtc_dev, &time_now, &valid));

        if (!valid)
        {
            ESP_LOGE(TAG, "RTC time invalid!");
            return;
        }

        ESP_LOGI(TAG, "Current RTC time: %02d:%02d",
                 time_now.tm_hour, time_now.tm_min);

        ESP_LOGI(TAG, "Next medicine time: %02d:%02d",
                 next_time.hour, next_time.minute);

        if (time_now.tm_hour == next_time.hour && time_now.tm_min == next_time.minute)
        {
            ESP_LOGI(TAG, "Alarm time reached! Triggering alarm...");
            turn_alarm(true);
            // Call lvgl screen to show alarm
        }

        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
