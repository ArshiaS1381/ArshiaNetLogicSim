/*
 * File: hal_joystick.c
 * Version: 1.0.0
 * Description:
 * Simulation stub for Joystick input.
 * Always returns CENTER and NOT PRESSED in this version.
 * Future versions will map to I2C or SPI ADC readings.
 */

#include "hal_joystick.h"
#include <stdio.h>
#include <stdbool.h>

void HAL_Joystick_Init(void) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for I2C/SPI ADC initialization
        printf("  [Joystick] Initialized (REAL HW)\n");
    #else
        // [SIMULATION]: Virtual stub initialization
        printf("  [Joystick] Initialized (Virtual)\n");
    #endif
}

JoystickDir HAL_Joystick_GetDir(void) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for reading real ADC values
        return JOY_CENTER; 
    #else
        // Stub: No movement
        return JOY_CENTER;
    #endif
}

bool HAL_Joystick_IsPressed(void) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for reading real button state
        return false;
    #else
        // Stub: Button not pressed
        return false;
    #endif
}