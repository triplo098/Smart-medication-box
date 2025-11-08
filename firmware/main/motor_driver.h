#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// Motor driver pins
#define INT1_PIN 1
#define INT2_PIN 3
#define INT3_PIN 8
#define INT4_PIN 43

// Mechanical parameters
#define NUMEBR_OF_SECTIONS 32
#define MOTOR_STEPS_PER_REV 2048                                                                       // For 28BYJ-48 stepper motor
#define MOTOR_REV_FOR_FULL_SECTIONS_REV 2 // 2:1 gear ration                                                              //
#define STEPS_PER_SECTION (MOTOR_STEPS_PER_REV * MOTOR_REV_FOR_FULL_SECTIONS_REV / NUMEBR_OF_SECTIONS) // 512 steps per section

// Initializes motor GPIOs
void init_motor();

// Sets motor to specific section (0 to NUMEBR_OF_SECTIONS-1)
esp_err_t set_section(unsigned int section);

// Moves n steps FreeRTOS task. Positive CW, Negative CCW.
void move_n_steps_task(void *pvParameter);

#endif // MOTOR_DRIVER_H