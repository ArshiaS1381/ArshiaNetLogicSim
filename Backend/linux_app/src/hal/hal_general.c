/*
 * File: hal_general.c
 * Version: 1.0.0
 * Description:
 * Master initialization module for the Hardware Abstraction Layer.
 * Calls individual init routines for subsystems (GPIO, LED, R5F, etc.).
 *
 * Note: In this version (1.0.0), this acts as a simulation stub wrapper.
 * Calls to hardware-specific drivers will print status messages to console
 * rather than accessing physical registers.
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
        printf("[HAL] Hardware Initialization\n");
    #else
        printf("[HAL] Hardware Initialization (Sim)\n");
    #endif
    
    Timer_Init();
    HAL_GPIO_Init();
    HAL_Joystick_Init();
    HAL_LED_Init();
    HAL_Rotary_Init();
    
    #ifdef BEAGLEY_BUILD
        printf("[HAL] Hardware Initialization Complete\n");
    #else
        printf("[HAL] Hardware Initialization Complete (Sim)\n");
    #endif
}

/*
 * Function: HAL_General_Cleanup
 * -----------------------------
 * Shuts down hardware subsystems.
 */
void HAL_General_Cleanup(void) {
    printf("[HAL] Shutting down hardware...\n");
    HAL_GPIO_Cleanup();
    // Add other cleanup calls if necessary
}

/*
 * Function: HAL_General_GetBoardTemp
 * ----------------------------------
 * Returns a simulated board temperature.
 */
float HAL_General_GetBoardTemp(void) {
    // Simulation stub: Return a fixed "safe" temperature
    return 45.5f;
}