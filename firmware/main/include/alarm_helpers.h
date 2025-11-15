#ifndef ALARM_HELPERS_H
#define ALARM_HELPERS_H

#include "driver/gpio.h"
#include <stdbool.h>

#define BUZZER_PIN 42

esp_err_t alarm_gpio_init();
void turn_alarm(bool turn);


#endif