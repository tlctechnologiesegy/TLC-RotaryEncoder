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

// Add button tracking variables
static uint32_t button_press_count = 0;
static uint32_t last_press_duration = 0;
static TickType_t press_start_time = 0;
static QueueHandle_t button_event_queue = NULL;

// Timer ISR to handle rotary encoder reading
bool IRAM_ATTR timer_isr_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    static int last_valid_state_a = -1;
    static int last_valid_state_b = -1;
    static TickType_t last_change_time = 0;
    
    int current_state_a = gpio_get_level(ROTARY_ENCODER_PIN_A);
    int current_state_b = gpio_get_level(ROTARY_ENCODER_PIN_B);
    BaseType_t high_task_wakeup = pdFALSE;
    TickType_t current_time = xTaskGetTickCountFromISR();

    // Fix time comparison
    TickType_t time_diff = current_time - last_change_time;
    if (time_diff >= pdMS_TO_TICKS(5)) {
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

// Remove old button task as it's replaced by button_monitor_task

// Button monitoring task
void button_monitor_task(void *pvParameter) {
    button_event_t event;
    static bool was_pressed = false;
    
    while (1) {
        bool is_pressed = (gpio_get_level(ROTARY_ENCODER_BUTTON_PIN) == 0); // Active low

        if (is_pressed && !was_pressed) {
            // Button just pressed
            press_start_time = xTaskGetTickCount();
            printf("Button pressed! Count: %lu\n", button_press_count);
        }
        else if (!is_pressed && was_pressed) {
            // Button just released
            TickType_t press_end_time = xTaskGetTickCount();
            last_press_duration = (press_end_time - press_start_time) * portTICK_PERIOD_MS;
            button_press_count++;
            
            printf("Button released! Duration: %lu ms, Total presses: %lu\n", 
                   last_press_duration, button_press_count);
                   
            // Prepare and send button event
            event.pressed = false;
            event.press_duration = last_press_duration;
            event.press_count = button_press_count;
            xQueueSend(button_event_queue, &event, 0);
        }
        
        was_pressed = is_pressed;
        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms polling interval
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

    // Remove old button task creation
    // Create button monitor task instead
    button_event_queue = xQueueCreate(5, sizeof(button_event_t));
    xTaskCreate(button_monitor_task, "ButtonMonitor", 2048, NULL, 5, NULL);

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
    return (gpio_get_level(ROTARY_ENCODER_BUTTON_PIN) == 0);
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

// Add new getter functions for button
uint32_t rotary_encoder_get_button_press_count(void) {
    return button_press_count;
}

uint32_t rotary_encoder_get_last_press_duration(void) {
    return last_press_duration;
}
