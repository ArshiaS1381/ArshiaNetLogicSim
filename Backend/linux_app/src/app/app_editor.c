/*
 * File: app_editor.c
 * Version: 1.1.0
 * Description:
 * Implements the interactive equation editor logic.
 * This module abstracts the manipulation of the text buffer and handles
 * the state machine for the Rotary/Joystick menu system (Requirement #4 & #7).
 */

#include "app_editor.h"
#include "logic_parser.h" // For syntax checking
#include "hal_joystick.h" // For JoystickDir and JOY_LEFT/RIGHT enums
#include <string.h>
#include <stdio.h>

// --- Internal State ---
static char editor_buffer[256];
static int cursor_pos = 0;

// Menu Definition
#define MENU_SIZE 10
static const char* MENU_ITEMS[MENU_SIZE] = {
    "A", "B", "C", "D", "E", "F", "OP", "DEL", "CLR", "SET"
};

// Menu State
static int menu_index = 0;       // Current position in MENU_ITEMS
static int op_sub_index = 0;     // 0=AND, 1=OR, 2=XOR (Used when menu_index is "OP")
static bool syntax_valid = false; // Cached validity status

/*
 * Internal Helper: check_syntax
 * -----------------------------
 * parses the current buffer to update the valid flag (controls Green LED).
 */
static void check_syntax(void) {
    LogicNode* root = Parser_ParseString(editor_buffer);
    if (root) {
        syntax_valid = true;
        AST_Free(root);
    } else {
        syntax_valid = false;
    }
}

/*
 * Function: Editor_Init
 * ---------------------
 * Resets the internal buffer, cursor, and menu state.
 */
void Editor_Init(void) {
    editor_buffer[0] = '\0';
    cursor_pos = 0;
    menu_index = 0;
    op_sub_index = 0;
    check_syntax();
}

/*
 * Function: Editor_LoadLine
 * -------------------------
 * Loads an existing string into the editor buffer for modification.
 */
void Editor_LoadLine(const char* current_text) {
    strncpy(editor_buffer, current_text, 255);
    editor_buffer[255] = '\0';
    cursor_pos = strlen(editor_buffer);
    check_syntax();
}

/*
 * Function: Editor_GetLine
 * ------------------------
 * Retrieves the current content of the editor buffer.
 */
const char* Editor_GetLine(void) {
    return editor_buffer;
}

/*
 * Function: Editor_GetMenuLabel
 * -----------------------------
 * Constructs a display string for the UI/Logs showing what is currently selected.
 */
const char* Editor_GetMenuLabel(void) {
    static char label_buf[32];
    
    if (strcmp(MENU_ITEMS[menu_index], "OP") == 0) {
        // Dynamic label for Operators
        const char* op_name = (op_sub_index == 0) ? "AND (*)" : 
                              (op_sub_index == 1) ? "OR (+)" : "XOR (^)";
        snprintf(label_buf, sizeof(label_buf), "OP: %s", op_name);
    } else {
        // Standard label
        snprintf(label_buf, sizeof(label_buf), "[ %s ]", MENU_ITEMS[menu_index]);
    }
    return label_buf;
}

/*
 * Function: Editor_IsSyntaxValid
 * ------------------------------
 * Returns the cached validity status.
 */
bool Editor_IsSyntaxValid(void) {
    return syntax_valid;
}

/*
 * Function: Editor_UpdateState
 * ----------------------------
 * Handles Rotary scrolling and Joystick sub-selection.
 * Updates the internal menu index based on hardware inputs.
 */
void Editor_UpdateState(int rotary_delta, JoystickDir joy_dir) {
    // 1. Handle Rotary (Menu Navigation)
    if (rotary_delta != 0) {
        menu_index += rotary_delta;
        
        // Wrap around logic
        if (menu_index < 0) menu_index = MENU_SIZE - 1;
        if (menu_index >= MENU_SIZE) menu_index = 0;
    }

    // 2. Handle Joystick (Sub-menu for OP)
    // Uses JOY_LEFT and JOY_RIGHT from hal_joystick.h
    if (strcmp(MENU_ITEMS[menu_index], "OP") == 0) {
        if (joy_dir == JOY_RIGHT) {
            op_sub_index++;
            if (op_sub_index > 2) op_sub_index = 0;
        } else if (joy_dir == JOY_LEFT) {
            op_sub_index--;
            if (op_sub_index < 0) op_sub_index = 2;
        }
    }
}

/*
 * Function: Editor_HandleButton
 * -----------------------------
 * Executes the action associated with the current menu item.
 * Handles Double Click logic for variables (Requirement #6).
 */
EditorResult Editor_HandleButton(RotaryButtonState btn_state) {
    if (btn_state == ROT_BTN_NONE) return EDITOR_RESULT_NONE;

    const char* item = MENU_ITEMS[menu_index];

    // --- Logic for "SET" (Save) ---
    if (strcmp(item, "SET") == 0) {
        return syntax_valid ? EDITOR_RESULT_SAVE : EDITOR_RESULT_NONE;
    }

    // --- Logic for "CLR" (Clear) ---
    if (strcmp(item, "CLR") == 0) {
        Editor_Clear();
        return EDITOR_RESULT_MODIFIED;
    }

    // --- Logic for "DEL" (Backspace) ---
    if (strcmp(item, "DEL") == 0) {
        Editor_Backspace();
        return EDITOR_RESULT_MODIFIED;
    }

    // --- Logic for "OP" (Operators) ---
    if (strcmp(item, "OP") == 0) {
        char op_char = (op_sub_index == 0) ? '*' : 
                       (op_sub_index == 1) ? '+' : '^';
        Editor_InsertChar(op_char);
        return EDITOR_RESULT_MODIFIED;
    }

    // --- Logic for Variables (A-F) ---
    // Check if it is a single char variable
    if (strlen(item) == 1 && item[0] >= 'A' && item[0] <= 'F') {
        Editor_InsertChar(item[0]);
        
        // Requirement #6: Double Press negates the variable just added
        if (btn_state == ROT_BTN_DOUBLE_CLICK) {
            Editor_InsertChar('\''); // Add NOT
        }
        return EDITOR_RESULT_MODIFIED;
    }

    return EDITOR_RESULT_NONE;
}

// --- Basic Buffer Operations ---

/*
 * Function: Editor_InsertChar
 * ---------------------------
 * Inserts a character and re-validates syntax.
 */
void Editor_InsertChar(char c) {
    if (cursor_pos < 255) {
        editor_buffer[cursor_pos++] = c;
        editor_buffer[cursor_pos] = '\0';
        check_syntax();
    }
}

/*
 * Function: Editor_Backspace
 * --------------------------
 * Removes a character and re-validates syntax.
 */
void Editor_Backspace(void) {
    if (cursor_pos > 0) {
        editor_buffer[--cursor_pos] = '\0';
        check_syntax();
    }
}

/*
 * Function: Editor_Clear
 * ----------------------
 * Clears the buffer and re-validates syntax.
 */
void Editor_Clear(void) {
    editor_buffer[0] = '\0';
    cursor_pos = 0;
    check_syntax();
}