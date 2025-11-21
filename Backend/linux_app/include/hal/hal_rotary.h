/*
 * File: hal_rotary.h
 * Version: 1.0.0
 * Description:
 * Hardware Abstraction Layer for a Rotary Encoder.
 * Used for scrolling through lists or adjusting numerical values.
 *
 * This module handles quadrature decoding to determine direction and count.
 */

#ifndef HAL_ROTARY_H
#define HAL_ROTARY_H

#include <stdbool.h>

// Enum for button events (Needed by app_editor)
typedef enum {
    ROT_BTN_NONE,
    ROT_BTN_CLICK,       // Single short press
    ROT_BTN_DOUBLE_CLICK // Two presses within 300ms
} RotaryButtonState;

/*
 * Function: HAL_Rotary_Init
 * -------------------------
 * Configures the interrupt pins or polling timer for the encoder.
 */
void HAL_Rotary_Init(void);

/*
 * Function: HAL_Rotary_Cleanup
 * -------------------------
 * Releases any resources associated with the rotary encoder.
 */
void HAL_Rotary_Cleanup(void);


/*
 * Function: HAL_Rotary_GetCount
 * -----------------------------
 * Returns the net change in position since the last call.
 *
 * returns: +1 for clockwise click, -1 for counter-clockwise, 0 for no change.
 */
int HAL_Rotary_GetCount(void);

/*
 * Function: HAL_Rotary_GetButtonEvent
 * ------------------------------
 * Reads the state of the push-button built into the rotary encoder.
 *
 * returns: true if the button is pressed.
 */
RotaryButtonState HAL_Rotary_GetButtonEvent(void);

#endif