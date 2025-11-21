/*
 * File: hal_led.h
 * Version: 1.0.0
 * Description:
 * Hardware Abstraction Layer for system LEDs.
 * Controls status indicators, such as the RGB system state LED
 * or individual channel activity LEDs.
 */

#ifndef HAL_LED_H
#define HAL_LED_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Function: HAL_LED_Init
 * ----------------------
 * Configures the PWM or GPIO channels used for driving LEDs.
 */
void HAL_LED_Init(void);

/*
 * Function: HAL_LED_SetRGB
 * ------------------------
 * Sets the color of the main system status LED.
 *
 * r: Red component (0-255).
 * g: Green component (0-255).
 * b: Blue component (0-255).
 */
void HAL_LED_SetRGB(uint8_t r, uint8_t g, uint8_t b);

/*
 * Function: HAL_LED_SetChannel
 * ----------------------------
 * Controls the discrete LED indicator for a specific logic channel.
 *
 * channel: Index of the channel (0-3 for X, Y, Z, W).
 * state:   true to turn ON, false to turn OFF.
 */
void HAL_LED_SetChannel(int channel, bool state);

#endif