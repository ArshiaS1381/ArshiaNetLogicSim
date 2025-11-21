/*
 * File: hal_gpio.h
 * Version: 1.0.0
 * Description:
 * Hardware Abstraction Layer (HAL) for General Purpose I/O.
 * Provides a standard interface to interact with the physical pins
 * on the host controller (e.g., reading switches, toggling debug lines).
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdbool.h>

/*
 * Enum: GPIOPin
 * -------------
 * Identifiers for the physical pins managed by this application.
 */
typedef enum {
    GPIO_SW1,
    GPIO_SW2,
    GPIO_DEBUG_0,
    GPIO_DEBUG_1
} GPIOPin;

/*
 * Function: HAL_GPIO_Init
 * -----------------------
 * Configures the memory mappings or file descriptors required to access
 * the GPIO hardware registers.
 */
void HAL_GPIO_Init(void);

/*
 * Function: HAL_GPIO_Read
 * -----------------------
 * Reads the current logic level of the specified input pin.
 *
 * pin:     The pin identifier to read.
 * returns: true for High (Logic 1), false for Low (Logic 0).
 */
bool HAL_GPIO_Read(GPIOPin pin);

/*
 * Function: HAL_GPIO_Write
 * ------------------------
 * Sets the logic level of the specified output pin.
 *
 * pin:   The pin identifier to write to.
 * value: true to set High, false to set Low.
 */
void HAL_GPIO_Write(GPIOPin pin, bool value);

/*
 * Function: HAL_GPIO_Cleanup
 * --------------------------
 * Releases any resources (e.g., munmap, close) associated with GPIO access.
 */
void HAL_GPIO_Cleanup(void);

#endif