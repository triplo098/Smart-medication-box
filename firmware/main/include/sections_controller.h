#ifndef SECTIONS_CONTROLLER_H
#define SECTIONS_CONTROLLER_H

#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>
#include "medicines_managment.h"


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


typedef struct section {
    uint8_t section_number;
    struct tm exact_time;
    bool filled;
}section;

void init_sections_controller();

// Sets motor to specific section (0 to NUMEBR_OF_SECTIONS-1)
esp_err_t set_section(unsigned int section);

// Moves n steps FreeRTOS task. Positive CW, Negative CCW.
void move_n_steps_task(void *pvParameter);

// Calculates steps needed to move from current_section to target_section
signed int section_to_steps(unsigned int target_section);

uint8_t get_next_section_number_for_medicine(medicine_t *medicine);

uint8_t get_next_target_section_number_for_time(struct tm exact_time);


#endif // SECTIONS_CONTROLLER_H