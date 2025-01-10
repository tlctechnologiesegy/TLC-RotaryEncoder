#ifndef ROTARY_ENCODER_MODULE_H
#define ROTARY_ENCODER_MODULE_H

#include <stdbool.h>
#include <stdint.h>

// Encoder properties
#define STEPS_PER_ROTATION 20
#define DEGREES_PER_STEP 18  // 360/20

typedef enum {
    DIRECTION_NONE,
    DIRECTION_CW,
    DIRECTION_CCW
} encoder_direction_t;

typedef struct {
    int counter;
    int steps;
    encoder_direction_t direction;
    int angle;
    int rotations;
    int menu_position; // Add menu position
} encoder_event_t;

// Add button event structure
typedef struct {
    bool pressed;            // Button state
    uint32_t press_duration; // Duration of press in ms
    uint32_t press_count;    // Number of times button was pressed
} button_event_t;

void rotary_encoder_init(void);
int rotary_encoder_read(void);
int rotary_encoder_read_button(void);
void rotary_encoder_timer_init(void);
int rotary_encoder_get_angle(void);
encoder_direction_t rotary_encoder_get_direction(void);
int rotary_encoder_get_rotations(void);
int rotary_encoder_get_step_counter(void);
int rotary_encoder_get_elapsed_steps(void);
int rotary_encoder_get_menu_position(void);

// Add new button functions
uint32_t rotary_encoder_get_button_press_count(void);
uint32_t rotary_encoder_get_last_press_duration(void);

#endif // ROTARY_ENCODER_MODULE_H
