/*
 * File: hal_gpio.h
 * Version: 1.1.0
 * Description:
 * Hardware Abstraction Layer (HAL) for General Purpose I/O.
 * Defines the logical pin names used throughout the application.
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdbool.h>

/*
 * Enum: GPIOPin
 * -------------
 * Identifiers for the physical pins managed by this application.
 * These map to specific GPIO lines in hal_gpio.c
 */
typedef enum {
    // Inputs (Switches/Jumpers for A-F)
    GPIO_IN_A = 0,
    GPIO_IN_B,
    GPIO_IN_C,
    GPIO_IN_D,
    GPIO_IN_E,
    GPIO_IN_F,
    
    // Outputs (LEDs/Results for X-W)
    GPIO_OUT_X,
    GPIO_OUT_Y,
    GPIO_OUT_Z,
    GPIO_OUT_W,

    // Sentinel value to determine array size
    GPIO_COUNT 
} GPIOPin;

/*
 * Function: HAL_GPIO_Init
 * -----------------------
 * Configures the memory mappings or file descriptors required to access
 * the GPIO hardware registers via libgpiod.
 */
void HAL_GPIO_Init(void);

/*
 * Function: HAL_GPIO_Read
 * -----------------------
 * Reads the current logic level of the specified input pin.
 * returns: true for High (Logic 1), false for Low (Logic 0).
 */
bool HAL_GPIO_Read(GPIOPin pin);

/*
 * Function: HAL_GPIO_Write
 * ------------------------
 * Sets the logic level of the specified output pin.
 * value: true to set High, false to set Low.
 */
void HAL_GPIO_Write(GPIOPin pin, bool value);

/*
 * Function: HAL_GPIO_Cleanup
 * --------------------------
 * Releases any resources associated with GPIO access.
 */
void HAL_GPIO_Cleanup(void);

#endif