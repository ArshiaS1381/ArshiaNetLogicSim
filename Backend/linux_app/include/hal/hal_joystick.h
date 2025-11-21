/*
 * File: hal_joystick.h
 * Version: 1.0.0
 * Description:
 * Hardware Abstraction Layer for an analog or digital joystick.
 * Used to navigate the on-screen menus or select logic gates.
 *
 * Abstracts the underlying ADC or GPIO reads into direction events.
 */

#ifndef HAL_JOYSTICK_H
#define HAL_JOYSTICK_H

#include <stdbool.h>

/*
 * Enum: JoystickDir
 * -----------------
 * Represents the discrete directions resolved from the joystick input.
 */
typedef enum {
    JOY_CENTER,
    JOY_UP,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT
} JoystickDir;

/*
 * Function: HAL_Joystick_Init
 * ---------------------------
 * Initializes the joystick hardware interface (e.g., I2C, SPI, or ADC).
 */
void HAL_Joystick_Init(void);

/*
 * Function: HAL_Joystick_GetDir
 * -----------------------------
 * Polls the joystick and returns its current directional state.
 * This function may include dead-zone logic to prevent drift.
 *
 * returns: The current JoystickDir enum value.
 */
JoystickDir HAL_Joystick_GetDir(void);

/*
 * Function: HAL_Joystick_IsPressed
 * --------------------------------
 * Checks if the integrated joystick push-button (Z-axis) is active.
 *
 * returns: true if the button is currently pressed.
 */
bool HAL_Joystick_IsPressed(void);

#endif