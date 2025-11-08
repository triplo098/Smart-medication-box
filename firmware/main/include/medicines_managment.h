#ifndef MEDICINES_MANAGMENT_H
#define MEDICINES_MANAGMENT_H

#include "time.h"
#define MAX_MEDICINES_TOTAL 20
#define MAX_MEDICINE_DOSES_NUMBER_PER_DAY 10

#define MAX_MEDICINE_NAME_LENGTH 64


typedef struct
{
    char name[MAX_MEDICINE_NAME_LENGTH];
    __uint16_t doses_per_day;
    struct tm dose_times[MAX_MEDICINE_DOSES_NUMBER_PER_DAY]; // Store only hour and minute fields
    char special_requirements[64]; // E.g., "Take after meals"
    __uint16_t treatment_days_left;

} medicine_t;



void get_next_medicine_time(struct tm *next_time);


void add_medicine(medicine_t medicine);

void save_medicines_to_storage(void);
void load_medicines_from_storage(void);


#endif // MEDICINES_MANAGMENT_H
