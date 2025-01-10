#include "rotary_encoder_module.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <stdio.h>

// Define GPIO pins for the rotary encoder
#define ROTARY_ENCODER_PIN_A 18
#define ROTARY_ENCODER_PIN_B 19
#define ROTARY_ENCODER_BUTTON_PIN 15

// Variables to track the state of the rotary encoder
static int last_state_a = 0;
static int last_state_b = 0;

// Additional tracking variables
static int complete_rotations = 0;
static encoder_direction_t current_direction = DIRECTION_NONE;
static int current_angle = 0;

// Add step counter
static int step_counter = 0;
static int elapsed_steps = 0;

// Add menu position counter
static int menu_position = 0;

// Variables to track the button state
static int button_state = 0;

// Add queue handle
static QueueHandle_t encoder_event_queue = NULL;

// Timer ISR to handle rotary encoder reading
bool IRAM_ATTR timer_isr_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    static int last_valid_state_a = -1;
    static int last_valid_state_b = -1;
    static TickType_t last_change_time = 0;
    
    int current_state_a = gpio_get_level(ROTARY_ENCODER_PIN_A);
    int current_state_b = gpio_get_level(ROTARY_ENCODER_PIN_B);
    BaseType_t high_task_wakeup = pdFALSE;
    TickType_t current_time = xTaskGetTickCountFromISR();

    // Only process if enough time has passed since last change
    if ((current_time - last_change_time) >= pdMS_TO_TICKS(5)) {
        if ((last_valid_state_a != current_state_a || last_valid_state_b != current_state_b) && 
            (current_state_a == last_state_a && current_state_b == last_state_b)) {
            
            encoder_event_t event;
            
            if (last_valid_state_a == 0 && current_state_a == 1) {
                if (current_state_b == 0) {
                    step_counter++;
                    menu_position++;
                    current_direction = DIRECTION_CW;
                } else {
                    step_counter--;
                    menu_position--;
                    current_direction = DIRECTION_CCW;
                }
                elapsed_steps++;

                // Calculate rotations and current angle based on step_counter
                complete_rotations = step_counter / STEPS_PER_ROTATION;
                int temp_steps = step_counter % STEPS_PER_ROTATION;
                if (temp_steps < 0) {
                    temp_steps += STEPS_PER_ROTATION;
                }
                current_angle = temp_steps * DEGREES_PER_STEP;

                // Prepare and send event
                event.counter = step_counter;
                event.steps = elapsed_steps;
                event.direction = current_direction;
                event.angle = current_angle;
                event.rotations = complete_rotations;
                event.menu_position = menu_position;

                xQueueSendFromISR(encoder_event_queue, &event, &high_task_wakeup);
                last_change_time = current_time;
            }
            
            last_valid_state_a = current_state_a;
            last_valid_state_b = current_state_b;
        }
    }

    last_state_a = current_state_a;
    last_state_b = current_state_b;

    return high_task_wakeup == pdTRUE;
}

// Add printer task
void encoder_printer_task(void *pvParameter) {
    encoder_event_t event;

    while (1) {
        if (xQueueReceive(encoder_event_queue, &event, portMAX_DELAY)) {
            printf("Direction: %s, Counter: %d, Total Steps: %d\n",
                   event.direction == DIRECTION_CW ? "CW" : "CCW",
                   event.counter, event.steps);
            printf("Angle: %d°, Rotations: %d, Menu Position: %d\n",
                   event.angle, event.rotations, event.menu_position);
        }
    }
}

// Task to check the button state
void rotary_encoder_task(void *pvParameter) {
    int last_button_state = button_state;

    while (1) {
        int button_pressed = rotary_encoder_read_button();

        if (button_pressed != last_button_state) {
            if (button_pressed) {
                printf("Button pressed!\n");
            } else {
                printf("Button released!\n");
            }
            last_button_state = button_pressed;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Adjust the delay as needed
    }
}

// Initialize the rotary encoder
void rotary_encoder_init(void) {
    // Create queue
    encoder_event_queue = xQueueCreate(10, sizeof(encoder_event_t));

    gpio_set_direction(ROTARY_ENCODER_PIN_A, GPIO_MODE_INPUT);
    gpio_set_direction(ROTARY_ENCODER_PIN_B, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ROTARY_ENCODER_PIN_A, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(ROTARY_ENCODER_PIN_B, GPIO_PULLUP_ONLY);
    gpio_set_direction(ROTARY_ENCODER_BUTTON_PIN, GPIO_MODE_INPUT);
    // Remove internal pull-up configuration for the button
    // gpio_set_pull_mode(ROTARY_ENCODER_BUTTON_PIN, GPIO_PULLUP_ONLY);

    // Initialize the last state variables
    last_state_a = gpio_get_level(ROTARY_ENCODER_PIN_A);
    last_state_b = gpio_get_level(ROTARY_ENCODER_PIN_B);

    // Initialize the hardware timer
    rotary_encoder_timer_init();

    // Create a FreeRTOS task to check the button state
    xTaskCreate(rotary_encoder_task, "RotaryEncoderTask", 2048, NULL, 5, NULL);

    // Create printer task
    xTaskCreate(encoder_printer_task, "EncoderPrinter", 2048, NULL, 1, NULL);
}

// Initialize the hardware timer
void rotary_encoder_timer_init(void) {
    gptimer_handle_t timer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 MHz, 1 us per tick
    };
    gptimer_new_timer(&timer_config, &timer);

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_isr_callback,
    };
    gptimer_register_event_callbacks(timer, &cbs, NULL);

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = 1000, // 1 ms
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(timer, &alarm_config);

    gptimer_enable(timer); // Ensure the timer is enabled before starting
    gptimer_start(timer);
}

// Read the button state with debounce
int rotary_encoder_read_button(void) {
    static int last_state = 1;
    static int last_debounce_time = 0;
    int current_state = gpio_get_level(ROTARY_ENCODER_BUTTON_PIN);
    if (current_state != last_state) {
        last_debounce_time = xTaskGetTickCount();
    }
    if ((xTaskGetTickCount() - last_debounce_time) > (50 / portTICK_PERIOD_MS)) {
        if (current_state != last_state) {
            last_state = current_state;
            if (current_state == 0) { // Active low: button pressed
                button_state = 1;
                return 1; // Button pressed
            }
        }
    }
    button_state = 0;
    return 0; // Button not pressed
}

// New getter functions
int rotary_encoder_get_angle(void) {
    return current_angle;
}

encoder_direction_t rotary_encoder_get_direction(void) {
    return current_direction;
}

int rotary_encoder_get_rotations(void) {
    return complete_rotations;
}

// Add getter functions for step counter and elapsed steps
int rotary_encoder_get_step_counter(void) {
    return step_counter;
}

int rotary_encoder_get_elapsed_steps(void) {
    return elapsed_steps;
}

// Add getter for menu position
int rotary_encoder_get_menu_position(void) {
    return menu_position;
}
