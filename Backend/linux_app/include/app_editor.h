/*
 * File: app_editor.h
 * Version: 1.1.0
 * Description:
 * Manages the on-screen line editor for entering logic equations.
 * This module abstracts the manipulation of the text buffer and implements
 * the state machine for the Rotary/Joystick menu system.
 */

#ifndef APP_EDITOR_H
#define APP_EDITOR_H

#include <stdbool.h>
#include "hal_rotary.h"   // For RotaryButtonState
#include "hal_joystick.h" // For JoystickDirection

// Return codes for the main loop to act upon
typedef enum {
    EDITOR_RESULT_NONE,
    EDITOR_RESULT_MODIFIED, // Content changed (Flash Red LED)
    EDITOR_RESULT_SAVE      // User selected "SET" (Save to AppState)
} EditorResult;

/*
 * Function: Editor_Init
 * ---------------------
 * Initializes the editor buffers and resets cursor/menu state.
 */
void Editor_Init(void);

/*
 * Function: Editor_LoadLine
 * -------------------------
 * Loads an existing string into the editor buffer.
 */
void Editor_LoadLine(const char* current_text);

/*
 * Function: Editor_GetLine
 * ------------------------
 * Retrieves the current content of the editor buffer.
 */
const char* Editor_GetLine(void);

/*
 * Function: Editor_GetMenuLabel
 * -----------------------------
 * Returns the text to display for the current rotary menu selection.
 * e.g., "A", "B", "OP: AND", "DEL", "SET"
 */
const char* Editor_GetMenuLabel(void);

/*
 * Function: Editor_IsSyntaxValid
 * ------------------------------
 * Returns true if the current buffer forms a valid logic equation.
 * Used to drive the Green LED.
 */
bool Editor_IsSyntaxValid(void);

/*
 * Function: Editor_UpdateState
 * ----------------------------
 * Called periodically by the main loop to update the menu selection.
 * * rotary_delta: Steps moved by the encoder (e.g., +1, -1).
 * joy_dir:      Current direction of the joystick (for sub-menus like OP).
 */
void Editor_UpdateState(int rotary_delta, JoystickDir joy_dir);

/*
 * Function: Editor_HandleButton
 * -----------------------------
 * Called when the Rotary button is pressed. Executes the selected action.
 * * btn_state: Single Click (Insert) or Double Click (Negate/Insert NOT).
 */
EditorResult Editor_HandleButton(RotaryButtonState btn_state);

// --- Legacy/Helper Functions (kept for internal use or UDP overrides) ---
void Editor_InsertChar(char c);
void Editor_Backspace(void);
void Editor_Clear(void);

#endif