/*
 * File: app_editor.c
 * Version: 1.0.0
 * Description:
 * Logic for the "Append-Only" equation editor.
 * This module maintains the text buffer for the currently active equation
 * being edited. It handles virtual cursor movement, safe character
 * insertion (bounds checking), and deletion.
 *
 * It is designed to abstract the input method, so it can be driven by
 * UDP commands, a physical rotary encoder, or a joystick.
 */

#include "app_editor.h"
#include <string.h>
#include <stdio.h>

// Internal buffer for the editor
static char editor_buffer[256];
static int cursor_pos = 0;

/*
 * Function: Editor_Init
 * ---------------------
 * Resets the internal buffer and cursor.
 */
void Editor_Init(void) {
    editor_buffer[0] = '\0';
    cursor_pos = 0;
}

/*
 * Function: Editor_LoadLine
 * -------------------------
 * Loads an existing string (e.g., a saved equation) into the editor.
 * Moves the cursor to the end of the line.
 */
void Editor_LoadLine(const char* current_text) {
    strncpy(editor_buffer, current_text, 255);
    editor_buffer[255] = '\0';
    cursor_pos = strlen(editor_buffer);
}

/*
 * Function: Editor_GetLine
 * ------------------------
 * Returns a pointer to the current buffer content.
 */
const char* Editor_GetLine(void) {
    return editor_buffer;
}

/*
 * Function: Editor_InsertChar
 * ---------------------------
 * Inserts a character at the current cursor position and advances it.
 * Includes safety checks to prevent buffer overflows.
 */
void Editor_InsertChar(char c) {
    if (cursor_pos < 255) {
        editor_buffer[cursor_pos++] = c;
        editor_buffer[cursor_pos] = '\0';
    }
}

/*
 * Function: Editor_Backspace
 * --------------------------
 * Removes the character preceding the cursor and moves the cursor back.
 * Safe to call even if the buffer is empty.
 */
void Editor_Backspace(void) {
    if (cursor_pos > 0) {
        editor_buffer[--cursor_pos] = '\0';
    }
}

/*
 * Function: Editor_Clear
 * ----------------------
 * Wipes the current line completely.
 */
void Editor_Clear(void) {
    editor_buffer[0] = '\0';
    cursor_pos = 0;
}