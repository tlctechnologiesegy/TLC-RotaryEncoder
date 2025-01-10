#ifndef ROTARY_ENCODER_MODULE_H
#define ROTARY_ENCODER_MODULE_H

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

#endif // ROTARY_ENCODER_MODULE_H
