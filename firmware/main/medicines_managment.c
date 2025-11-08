#include "medicines_managment.h"
#include "pcf8563.h"
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include "esp_log.h"

medicine_t medicines_list[MAX_MEDICINES_TOTAL] = {0};

extern i2c_dev_t rtc_dev;


static const char* TAG = "MED_MGMT";

void get_next_medicine_time(struct tm *next_time)
{
    ESP_LOGI(TAG, "Calculating next medicine time...");
    struct tm time_now;
    bool valid;
    ESP_ERROR_CHECK(pcf8563_get_time(&rtc_dev, &time_now, &valid));
    ESP_LOGI(TAG, "Current time: %02d:%02d", time_now.tm_hour, time_now.tm_min);

    time_t current_time = mktime(&time_now);
    time_t nearest_time = LONG_MAX;
    time_t smallest_diff = LONG_MAX;

    if (!valid)
    {   
        ESP_LOGE(TAG, "RTC time is not valid");
        *next_time = (struct tm){0};
        return;
    }

    for (int i = 0; i < MAX_MEDICINES_TOTAL; i++)
    {
        if (medicines_list[i].doses_per_day == 0)
        {   
            // ESP_LOGI(TAG, "Skipping empty medicine slot at index %d", i);
            continue; // Skip empty slots
        }

        ESP_LOGI(TAG, "Checking medicine: %s", medicines_list[i].name);
        
        for (int j = 0; j < medicines_list[i].doses_per_day; j++)
        {
            

            struct tm dose_time = medicines_list[i].dose_times[j];
            dose_time.tm_year = time_now.tm_year;
            dose_time.tm_mon = time_now.tm_mon;
            dose_time.tm_mday = time_now.tm_mday;

            time_t dose_time_t = mktime(&dose_time);
            double diff = difftime(dose_time_t, current_time);
            if (diff > 0 && (smallest_diff == -1 || diff < smallest_diff))
            {
                smallest_diff = diff;
                nearest_time = dose_time_t;
            }
        }
    }

    if (nearest_time != -1)
    {
        ESP_LOGI(TAG, "Next medicine time found");
        
        struct tm *result_time = localtime(&nearest_time);
        *next_time = *result_time;

    }
    else
    {
        ESP_LOGE(TAG, "No upcoming medicine times found");
        *next_time = (struct tm){0};
    }
}

void add_medicine(medicine_t medicine)
{   
    ESP_LOGI(TAG, "Adding medicine: %s", medicine.name);
    for (int i = 0; i < MAX_MEDICINES_TOTAL; i++)
    {
        if (medicines_list[i].doses_per_day == 0)
        {
            ESP_LOGI(TAG, "Found empty slot at index %d", i);
            medicines_list[i] = medicine; // Add medicine to the first empty slot

            break;
        }
    }
}

void remove_medicine(const char* name)
{
    ESP_LOGI(TAG, "Removing medicine: %s", name);
    for (int i = 0; i < MAX_MEDICINES_TOTAL; i++)
    {
        if (strcmp(medicines_list[i].name, name) == 0)
        {
            ESP_LOGI(TAG, "Medicine found at index %d, removing...", i);
            medicines_list[i] = (medicine_t){0}; // Clear the slot
            break;
        }
    }
}
