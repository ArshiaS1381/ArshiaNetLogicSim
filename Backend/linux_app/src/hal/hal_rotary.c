/*
 * File: hal_rotary.c
 * Version: 1.0.0
 * Description:
 * Simulation stub for Rotary Encoder input.
 * Always returns 0 (no movement) for now.
 */

#include "hal_rotary.h"
#include <stdio.h>
#include <stdbool.h>

void HAL_Rotary_Init(void) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for initializing rotary encoder pins/interrupts
        printf("  [Rotary] Initialized (REAL HW)\n");
    #else
        // [SIMULATION]: Virtual stub initialization
        printf("  [Rotary] Initialized (Virtual)\n");
    #endif
}

int HAL_Rotary_GetCount(void) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for reading real rotary counter state
        return 0;
    #else
        return 0;
    #endif
}

bool HAL_Rotary_GetButton(void) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for reading real rotary button state
        return false;
    #else
        return false;
    #endif
}