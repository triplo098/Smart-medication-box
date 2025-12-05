#include "sections_controller.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include <limits.h>

static const char *TAG = "sections_controller";

unsigned int current_step = 0;
unsigned int current_section = 0;

// Current time
extern struct tm current_time;
extern section sections[NUMEBR_OF_SECTIONS];




static uint8_t setps_sequence[4][4] = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}};

TaskHandle_t move_motor_task_handle = NULL;

signed int section_to_steps(unsigned int target_section)
{
    ESP_LOGI(TAG, "Calculating steps from section %d to %d", current_section, target_section);

    signed int diff_to_target_section = target_section - current_section;

    if (diff_to_target_section < (-1 * NUMEBR_OF_SECTIONS / 2))
    {
        diff_to_target_section %= NUMEBR_OF_SECTIONS;
    }
    else if (diff_to_target_section > (NUMEBR_OF_SECTIONS / 2))
    {
        diff_to_target_section = -1 * (NUMEBR_OF_SECTIONS - diff_to_target_section);
    }
    
    return diff_to_target_section * STEPS_PER_SECTION;
}

void init_motor()
{
    gpio_reset_pin(INT1_PIN);
    gpio_reset_pin(INT2_PIN);
    gpio_reset_pin(INT3_PIN);
    gpio_reset_pin(INT4_PIN);

    gpio_set_direction(INT1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(INT2_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(INT3_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(INT4_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(INT1_PIN, 0);
    gpio_set_level(INT2_PIN, 0);
    gpio_set_level(INT3_PIN, 0);
    gpio_set_level(INT4_PIN, 0);

    ESP_LOGI(TAG, "Motors GPIOs initialized");
}

static void set_step(unsigned int step_idx)
{
    gpio_set_level(INT1_PIN, setps_sequence[step_idx][0]);
    gpio_set_level(INT2_PIN, setps_sequence[step_idx][1]);
    gpio_set_level(INT3_PIN, setps_sequence[step_idx][2]);
    gpio_set_level(INT4_PIN, setps_sequence[step_idx][3]);
}

void move_n_steps_task(void *pvParameter)
{

    signed int n = (signed int)pvParameter;
    ESP_LOGI(TAG, "N: %d", n);
    ESP_LOGI(TAG, "Curent step: %d", current_step);

    bool clockwise = (n >= 0) ? true : false;
    n = abs(n);

    unsigned int step_idx = current_step % 4;

    for (unsigned int i = 0; i < n; i++)
    {

        set_step(step_idx);
        vTaskDelay(pdMS_TO_TICKS(10));

        current_step += (clockwise) ? 1 : -1;

        step_idx = current_step % 4;

        // ESP_LOGI(TAG, "Current step: %d, Step_idx: %d", current_step, step_idx);
        // ESP_LOGI(TAG, "Step: %d", current_step);
    }

    gpio_set_level(INT1_PIN, 0);
    gpio_set_level(INT2_PIN, 0);
    gpio_set_level(INT3_PIN, 0);
    gpio_set_level(INT4_PIN, 0);

    move_motor_task_handle = NULL;
    vTaskDelete(NULL);
}

esp_err_t set_section(unsigned int target_section)
{

    if (target_section >= NUMEBR_OF_SECTIONS)
    {
        ESP_LOGW(TAG, "Invalid section: %d", target_section);
        return ESP_ERR_INVALID_ARG;
    }

    signed int target_steps = section_to_steps(target_section);

    ESP_LOGI(TAG, "current: %d, target: %d, section move: %d, target_steps: %d", current_section, target_section, (target_steps / STEPS_PER_SECTION), target_steps);

    if (target_steps == 0)
    {
        return ESP_OK;
    }

    if (move_motor_task_handle == NULL)
    {
        xTaskCreate(move_n_steps_task, "move_motor_task", 2048, (void *)target_steps, 4, &move_motor_task_handle);
    }
    else
    {
        return ESP_ERR_INVALID_STATE;
    }

    current_section = target_section;

    return ESP_OK;
}

void init_sections_controller()
{   
    init_motor();

    for (int i = 0; i < NUMEBR_OF_SECTIONS; i++)
    {
        sections[i].section_number = i;
        sections[i].filled = false;
    }
}

static int tmcmp(struct tm t1, struct tm t2)
{
    if (t1.tm_year != t2.tm_year)
        return (t1.tm_year < t2.tm_year) ? -1 : 1;
    if (t1.tm_mon != t2.tm_mon)
        return (t1.tm_mon < t2.tm_mon) ? -1 : 1;
    if (t1.tm_mday != t2.tm_mday)
        return (t1.tm_mday < t2.tm_mday) ? -1 : 1;
    if (t1.tm_hour != t2.tm_hour)
        return (t1.tm_hour < t2.tm_hour) ? -1 : 1;
    if (t1.tm_min != t2.tm_min)
        return (t1.tm_min < t2.tm_min) ? -1 : 1;
    if (t1.tm_sec != t2.tm_sec)
        return (t1.tm_sec < t2.tm_sec) ? -1 : 1;
    return 0;
}


uint8_t get_next_target_section_number_for_time(struct tm exact_time)
{

    uint8_t next_section = 0;

    // Check for filled
    for (int i = 0; i < NUMEBR_OF_SECTIONS; i++)
    {   


        section sec = sections[next_section];
        // Prinitn seciotn info
        ESP_LOGI(TAG, "Checking section %d: filled=%d, time=%04d-%02d-%02d %02d:%02d:%02d",
                 sec.section_number,
                 sec.filled,
                 sec.exact_time.tm_year + 1900,
                 sec.exact_time.tm_mon + 1,
                 sec.exact_time.tm_mday,
                 sec.exact_time.tm_hour,
                 sec.exact_time.tm_min,
                 sec.exact_time.tm_sec);
        

        // Check if section is empty or occupied for the given time
        if (tmcmp(sec.exact_time, exact_time) == 0)
        {   
            ESP_LOGI(TAG, "Found exact section match, number: %d", sec.section_number);
            sec.exact_time = exact_time;
            return next_section;
        }

        next_section = (next_section + 1) % NUMEBR_OF_SECTIONS;
    }

    // Check for empty seciton
    for (int i = 0; i < NUMEBR_OF_SECTIONS; i++)
    {
        section sec = sections[next_section];

        // Check if section is empty or occupied for the given time
        if (sec.filled == false || 
        (sec.exact_time.tm_year == 0  && sec.exact_time.tm_mon == 0 && sec.exact_time.tm_mday == 0))
        {
            ESP_LOGI(TAG, "Found target section: %d", next_section);
         
            sec.exact_time = exact_time;
            return next_section;
        }
        ESP_LOGI(TAG, "section info: %d, %d, %d", sec.exact_time, sec.filled, sec.section_number);
        next_section = (next_section + 1) % NUMEBR_OF_SECTIONS;
    }



    ESP_LOGW(TAG, "No empty sections available!");
    return UINT8_MAX;
}
