#include "alarm_helpers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "ALARM";

static volatile bool alarmEnabled = false;
static TaskHandle_t alarmTaskHandle = NULL;

esp_err_t alarm_gpio_init()
{   
    ESP_LOGI(TAG, "Initialazing alarm");

    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_PIN, 0);
    return ESP_OK;
}

static void alarm_task(void *arg)
{
    int max_cycles = 20;

    for (int i = 0; i < max_cycles; i++)
    {
        if (!alarmEnabled) break;

        // Alarm sequence
        gpio_set_level(BUZZER_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(BUZZER_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(BUZZER_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(100));

        gpio_set_level(BUZZER_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(900));
    }

    // Ensure buzzer is off
    gpio_set_level(BUZZER_PIN, 0);

    // Clear handle BEFORE deleting task
    alarmTaskHandle = NULL;

    vTaskDelete(NULL);
}

void turn_alarm(bool turn)
{
    alarmEnabled = turn;

    if (turn)
    {
        // Prevent duplicate tasks
        if (alarmTaskHandle == NULL)
        {
            xTaskCreate(
                alarm_task,
                "alarm_task",
                2048,
                NULL,
                2,
                &alarmTaskHandle
            );
            ESP_LOGI(TAG, "Alarm started");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Alarm stopping...");
    }
}
