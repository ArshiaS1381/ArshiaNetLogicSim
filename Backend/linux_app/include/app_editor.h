/*
 * File: app_editor.h
 * Version: 1.0.0
 * Description:
 * Manages the on-screen line editor for entering logic equations.
 * This module abstracts the manipulation of the text buffer, handling
 * cursor movement, character insertion, and deletion.
 *
 * It separates the UI mechanics of typing from the logic of compiling.
 */

#ifndef APP_EDITOR_H
#define APP_EDITOR_H

#include <stdbool.h>

/*
 * Function: Editor_Init
 * ---------------------
 * Initializes the editor buffers and resets the cursor position.
 */
void Editor_Init(void);

/*
 * Function: Editor_LoadLine
 * -------------------------
 * Loads an existing string into the editor buffer for modification.
 *
 * current_text: The string to load (e.g., the current equation for X).
 */
void Editor_LoadLine(const char* current_text);

/*
 * Function: Editor_GetLine
 * ------------------------
 * Retrieves the current content of the editor buffer.
 *
 * returns: A pointer to the internal null-terminated buffer.
 */
const char* Editor_GetLine(void);

/*
 * Function: Editor_InsertChar
 * ---------------------------
 * Inserts a single character at the current cursor position.
 * Ignores input if the buffer is full.
 *
 * c: The character to insert.
 */
void Editor_InsertChar(char c);

/*
 * Function: Editor_Backspace
 * --------------------------
 * Removes the character immediately preceding the cursor.
 * Handles bounds checking to prevent underflow.
 */
void Editor_Backspace(void);

/*
 * Function: Editor_Clear
 * ----------------------
 * Empties the editor buffer completely.
 */
void Editor_Clear(void);

#endif