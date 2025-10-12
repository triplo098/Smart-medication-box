#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define NUMEBR_OF_SECTIONS 32
#define MOTOR_STEPS_PER_REV 2048 // For 28BYJ-48 stepper motor
#define STEPS_PER_SECTION (MOTOR_STEPS_PER_REV / NUMEBR_OF_SECTIONS) // 2048 / 32 = 64 steps per section


void init_motor();
esp_err_t set_section(unsigned int section);

// Moves n steps FreeRTOS task. Positive CW, Negative CCW.
void move_n_steps_task(void *pvParameter);

#endif // MOTOR_DRIVER_H