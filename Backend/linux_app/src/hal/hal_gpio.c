/*
 * File: hal_gpio.c
 * Version: 1.0.0
 * Description:
 * Simulation stub for GPIO control.
 * In a real deployment, this would map to /dev/gpiod or memory-mapped registers.
 * Currently, it maintains an internal state array to simulate pin values.
 */

#include "hal_gpio.h"
#include <stdio.h>

// Simulated hardware state
static bool pin_states[4] = { false, false, false, false };

void HAL_GPIO_Init(void) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for real hardware initialization
        printf("  [GPIO] Initialized (REAL HW)\n");
    #else
        // [SIMULATION]: Virtual stub initialization
        printf("  [GPIO] Initialized (Virtual)\n");
    #endif
}

bool HAL_GPIO_Read(GPIOPin pin) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for reading real GPIO register
        return false;
    #else
        // [SIMULATION]: Read from the simulated state array
        if (pin < 0 || pin > 3) return false;
        return pin_states[pin];
    #endif
}

void HAL_GPIO_Write(GPIOPin pin, bool value) {
    #ifdef BEAGLEY_BUILD
        // [BEAGLEY_BUILD]: Placeholder for writing to real GPIO register
    #else
        // [SIMULATION]: Write to the simulated state array
        if (pin < 0 || pin > 3) return;
        pin_states[pin] = value;
        // printf("  [GPIO] Pin %d set to %s\n", pin, value ? "HIGH" : "LOW");
    #endif
}

void HAL_GPIO_Cleanup(void) {
    // Nothing to clean up in simulation
}