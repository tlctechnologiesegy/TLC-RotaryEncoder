#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "rotary_encoder_module.h"

void app_main(void)
{
    printf("Hello, world!\n");

    rotary_encoder_init();

    // The task created in rotary_encoder_init will handle the encoder and button checks
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Main loop delay
    }
}
