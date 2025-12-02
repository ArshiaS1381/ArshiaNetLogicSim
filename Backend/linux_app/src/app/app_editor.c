/*
 * File: app_editor.c
 * Version: 1.3.0
 * Description:
 * Implements the interactive equation editor.
 * Uses '%' for NAND and '$' for NOR.
 */

#include "app_editor.h"
#include "logic_parser.h" 
#include "hal_joystick.h" 
#include <string.h>
#include <stdio.h>

static char editor_buffer[256];
static int cursor_pos = 0;

// Menu: Set, Clear, Delete, Logic, A-F
#define MENU_SIZE 10
static const char* MENU_ITEMS[MENU_SIZE] = {
    "SET", "CLR", "DEL", "LOGIC", "A", "B", "C", "D", "E", "F"
};

// Logic Sub-menu
#define OP_COUNT 5
static const char* OPS[OP_COUNT] = { "AND", "NOT", "OR", "NAND", "NOR" };

static int menu_index = 0;       
static int op_sub_index = 0;     
static bool syntax_valid = false; 

static void check_syntax(void) {
    LogicNode* root = Parser_ParseString(editor_buffer);
    if (root) {
        syntax_valid = true;
        AST_Free(root);
    } else {
        syntax_valid = false;
    }
}

void Editor_Init(void) {
    editor_buffer[0] = '\0';
    cursor_pos = 0;
    menu_index = 0;
    op_sub_index = 0; 
    check_syntax();
}

void Editor_LoadLine(const char* current_text) {
    strncpy(editor_buffer, current_text, 255);
    editor_buffer[255] = '\0';
    cursor_pos = strlen(editor_buffer);
    check_syntax();
}

const char* Editor_GetLine(void) { return editor_buffer; }

const char* Editor_GetMenuLabel(void) {
    static char label_buf[32];
    
    if (strcmp(MENU_ITEMS[menu_index], "LOGIC") == 0) {
        const char* symbol = "";
        switch(op_sub_index) {
            case 0: symbol = "*"; break;   // AND
            case 1: symbol = "'"; break;   // NOT
            case 2: symbol = "+"; break;   // OR
            case 3: symbol = "%"; break;   // NAND (Internal token)
            case 4: symbol = "$"; break;   // NOR (Internal token)
        }
        snprintf(label_buf, sizeof(label_buf), "LOGIC: %s (%s)", OPS[op_sub_index], symbol);
    } else {
        snprintf(label_buf, sizeof(label_buf), "[ %s ]", MENU_ITEMS[menu_index]);
    }
    return label_buf;
}

bool Editor_IsSyntaxValid(void) { return syntax_valid; }

void Editor_UpdateState(int rotary_delta, JoystickDir joy_dir) {
    // 1. Rotary: Main Menu
    if (rotary_delta != 0) {
        menu_index += rotary_delta;
        while (menu_index < 0) menu_index += MENU_SIZE;
        while (menu_index >= MENU_SIZE) menu_index -= MENU_SIZE;
    }

    // 2. Joystick: Logic Sub-menu
    if (strcmp(MENU_ITEMS[menu_index], "LOGIC") == 0) {
        if (joy_dir == JOY_RIGHT) {
            op_sub_index++;
            if (op_sub_index >= OP_COUNT) op_sub_index = 0;
        } else if (joy_dir == JOY_LEFT) {
            op_sub_index--;
            if (op_sub_index < 0) op_sub_index = OP_COUNT - 1;
        }
    }
}

EditorResult Editor_HandleButton(RotaryButtonState btn_state) {
    if (btn_state == ROT_BTN_NONE) return EDITOR_RESULT_NONE;
    const char* item = MENU_ITEMS[menu_index];

    if (strcmp(item, "SET") == 0) return syntax_valid ? EDITOR_RESULT_SAVE : EDITOR_RESULT_NONE;
    
    if (strcmp(item, "CLR") == 0) {
        Editor_Clear();
        return EDITOR_RESULT_MODIFIED;
    }
    if (strcmp(item, "DEL") == 0) {
        Editor_Backspace();
        return EDITOR_RESULT_MODIFIED;
    }

    if (strcmp(item, "LOGIC") == 0) {
        switch(op_sub_index) {
            case 0: Editor_InsertChar('*'); break; // AND
            case 1: Editor_InsertChar('\''); break; // NOT
            case 2: Editor_InsertChar('+'); break; // OR
            case 3: Editor_InsertChar('%'); break; // NAND
            case 4: Editor_InsertChar('$'); break; // NOR
        }
        return EDITOR_RESULT_MODIFIED;
    }

    if (strlen(item) == 1 && item[0] >= 'A' && item[0] <= 'F') {
        Editor_InsertChar(item[0]);
        return EDITOR_RESULT_MODIFIED;
    }

    return EDITOR_RESULT_NONE;
}

void Editor_InsertChar(char c) {
    if (cursor_pos < 255) {
        editor_buffer[cursor_pos++] = c;
        editor_buffer[cursor_pos] = '\0';
        check_syntax();
    }
}
void Editor_Backspace(void) {
    if (cursor_pos > 0) {
        editor_buffer[--cursor_pos] = '\0';
        check_syntax();
    }
}
void Editor_Clear(void) {
    editor_buffer[0] = '\0';
    cursor_pos = 0;
    check_syntax();
}