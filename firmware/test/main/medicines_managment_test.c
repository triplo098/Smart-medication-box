#include "unity.h"
#include "medicines_managment.h"
#include "alarm_helpers.h"
#include <string.h>

static void print_banner(const char *text);

TEST_CASE("Add Medicine Test", "[medicines]")
{
        medicine_list_init();
        struct tm time = {0};

        medicine_t *med = heap_caps_malloc(sizeof(medicine_t), MALLOC_CAP_DEFAULT);
        
        strcpy(med->name, "MED");
        med->doses_per_day = 1;
        med->dose_times[0].hour = 8;
        med->dose_times[0].minute = 0;
        strcpy(med->special_requirements, "None");
        med->treatment_start_date = time;
        med->treatment_end_date = time;
        
        add_medicine(med);
        medicine_t *retrieved_med = get_medicine_from_name(med->name);

        TEST_ASSERT_NOT_NULL(retrieved_med);
        TEST_ASSERT_EQUAL_STRING(med->name, "MED");
}

TEST_CASE("Remove Medicine Test", "[medicines]")
{
        medicine_list_init();
        struct tm time = {0};

        medicine_t *med = heap_caps_malloc(sizeof(medicine_t), MALLOC_CAP_DEFAULT);
        
        strcpy(med->name, "MED");
        med->doses_per_day = 1;
        med->dose_times[0].hour = 8;
        med->dose_times[0].minute = 0;
        strcpy(med->special_requirements, "None");
        med->treatment_start_date = time;
        med->treatment_end_date = time;
        
        add_medicine(med);        
        remove_medicine(med->name);

        medicine_t *deleted_med = get_medicine_from_name(med->name);
        TEST_ASSERT_NULL(deleted_med);
}

TEST_CASE("Get next dose time", "[medicines]")
{
        medicine_list_init();

        extern struct tm current_time;      
        current_time.tm_year = 2025 - 1900; // 2025
        current_time.tm_mon = 12 - 1;       // December
        current_time.tm_mday = 1;           // 1st
        current_time.tm_hour = 7;           // 7 AM
        current_time.tm_min = 30;           // 30 minutes


        medicine_t *med = heap_caps_malloc(sizeof(medicine_t), MALLOC_CAP_DEFAULT);
        strcpy(med->name, "MED");
        med->doses_per_day = 2;
        med->dose_times[0].hour = 8;
        med->dose_times[0].minute = 0;
        med->dose_times[1].hour = 12;
        med->dose_times[1].minute = 0;
        strcpy(med->special_requirements, "None");

        med->treatment_start_date.tm_year = 2025 - 1900;
        med->treatment_start_date.tm_mon = 12 - 1;
        med->treatment_start_date.tm_mday = 1;

        med->treatment_end_date.tm_year = 2025 - 1900;
        med->treatment_end_date.tm_mon = 12 - 1;
        med->treatment_end_date.tm_mday = 31;

        add_medicine(med);

        time_mh_t next_time;
        get_next_medicine_time(&next_time);

        TEST_ASSERT_EQUAL_UINT8(8, next_time.hour);
        TEST_ASSERT_EQUAL_UINT8(0, next_time.minute);
}

void app_main(void)
{
        print_banner("Running all the registered tests");
        UNITY_BEGIN();
        unity_run_all_tests();
        UNITY_END();

        print_banner("Starting interactive test menu");
        unity_run_menu();
}

static void print_banner(const char *text)
{
        printf("\n#### %s #####\n\n", text);
}