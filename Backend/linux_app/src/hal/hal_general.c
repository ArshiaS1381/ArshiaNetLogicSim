/*
 * File: hal_general.c
 * Version: 1.1.0
 * Description:
 * Master initialization module. Brings up GPIO, Joystick, LED, and Rotary subsystems.
 */

#include "hal_general.h"
#include "hal_gpio.h"
#include "hal_joystick.h"
#include "hal_led.h"
#include "hal_rotary.h"
#include "utils_timer.h"
#include <stdio.h>

/*
 * Function: HAL_General_Init
 * --------------------------
 * Brings up all hardware subsystems in the correct order.
 */
void HAL_General_Init(void) {
    #ifdef BEAGLEY_BUILD
        printf("[HAL] Hardware Initialization (Real)\n");
    #else
        printf("[HAL] Hardware Initialization (Sim)\n");
    #endif
    
    Timer_Init();
    HAL_GPIO_Init();     // Setup libgpiod lines
    HAL_Joystick_Init(); // Setup SPI
    HAL_LED_Init();      // Setup Sysfs
    HAL_Rotary_Init();   // Start Decoder Thread
    
    printf("[HAL] Init Complete.\n");
}

void HAL_General_Cleanup(void) {
    printf("[HAL] Shutting down hardware...\n");
    HAL_Rotary_Cleanup();
    HAL_GPIO_Cleanup();
    // Joystick/LED don't strictly require cleanup (OS handles fds)
}

/*
 * Function: HAL_General_GetBoardTemp
 * ----------------------------------
 * Returns a simulated board temperature.
 */
float HAL_General_GetBoardTemp(void) {
    // Stub
    return 40.0f;
}