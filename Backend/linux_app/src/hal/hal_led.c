/*
 * File: hal_led.c
 * Version: 1.0.0
 * Description:
 * Simulation stub for LED control.
 * Prints console messages when LED states change instead of driving physical pins.
 */

#include "hal_led.h"
#include <stdio.h>
#include <stdbool.h>

void HAL_LED_Init(void) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for initializing PWM/GPIO for LEDs
        printf("  [LED] Initialized (REAL HW)\n");
    #else
        // [SIMULATION]: Virtual stub initialization
        printf("  [LED] Initialized (Virtual)\n");
    #endif
}

void HAL_LED_SetRGB(uint8_t r, uint8_t g, uint8_t b) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for setting real RGB PWM values
    #else
        // Prints console messages in simulation
        // printf("  [LED] Main RGB set to (%d, %d, %d)\n", r, g, b);
    #endif
}

void HAL_LED_SetChannel(int channel, bool state) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for setting real channel GPIO/PWM
    #else
        // Prints console messages in simulation
        // printf("  [LED] Channel %d indicator: %s\n", channel, state ? "ON" : "OFF");
    #endif
}