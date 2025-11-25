#ifndef MEDICINES_MANAGMENT_H
#define MEDICINES_MANAGMENT_H

#include "time.h"
#include "freertos/FreeRTOS.h"

#define MAX_MEDICINES_TOTAL 20
#define MAX_MEDICINE_DOSES_NUMBER_PER_DAY 10

#define MAX_MEDICINE_STRING_LENGTH 64

typedef struct
{
    uint8_t hour;   // 0–23
    uint8_t minute; // 0–59
} time_mh_t;

typedef struct
{
    char name[MAX_MEDICINE_STRING_LENGTH];
    uint16_t doses_per_day;
    time_mh_t dose_times[MAX_MEDICINE_DOSES_NUMBER_PER_DAY];
    char special_requirements[MAX_MEDICINE_STRING_LENGTH]; // E.g., "Take after meals"
    struct tm treatment_start_date;
    struct tm treatment_end_date;
} medicine_t;

void get_next_medicine_time(time_mh_t *next_time);

void add_medicine(medicine_t *medicine);
void remove_medicine(const char* medicine_name);

void save_medicines_to_storage(void);
void load_medicines_from_storage(void);

void check_for_alarm_task(void *arg);

#endif // MEDICINES_MANAGMENT_H
