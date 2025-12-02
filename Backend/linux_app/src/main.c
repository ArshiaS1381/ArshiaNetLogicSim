/*
 * File: main.c
 * Version: 1.4.0
 * Description:
 * Main loop with "Queued: ..." feedback for interactive editing.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "app_state.h"
#include "app_editor.h"
#include "app_utils.h"
#include "net_udp.h"
#include "hal_general.h"
#include "hal_gpio.h"
#include "hal_joystick.h"
#include "hal_rotary.h"
#include "hal_led.h"
#include "utils_colors.h"
#include "utils_timer.h"
#include "logic_parser.h"
#include "logic_ast.h" 

const char* get_mode_name(SystemMode m) {
    switch(m) {
        case MODE_PROGRAM_X: return "PRG X";
        case MODE_PROGRAM_Y: return "PRG Y";
        case MODE_PROGRAM_Z: return "PRG Z";
        case MODE_PROGRAM_W: return "PRG W";
        case MODE_ROTARY_EXEC: return "RUN (ROT)";
        case MODE_GPIO_EXEC: return "RUN (GPIO)";
        default: return "UNKNOWN";
    }
}

// Run (Rot) Menu
static int run_menu_index = 0;
static const char* RUN_MENU_ITEMS[] = { "A", "B", "C", "D", "E", "F" };

// Buffer to track the last printed menu label to prevent console spam
static char last_print_buf[64] = "";

int main() {
    AppState_Init();
    Editor_Init();
    HAL_General_Init();
    NetUDP_Init();

    printf(C_B_GREEN "=== LOGIC SIM ENGINE STARTED ===" C_RESET "\n");

    SystemMode current_mode = MODE_PROGRAM_X;
    JoystickDir last_joy = JOY_CENTER;
    
    long long led_flash_start = 0;
    bool flash_active = false;

    while (1) {
        if (NetUDP_ExitRequested()) break;

        // 1. Polling
        JoystickDir joy = HAL_Joystick_GetDir();
        int rot_delta = HAL_Rotary_GetCount();
        RotaryButtonState rot_btn = HAL_Rotary_GetButtonEvent();

        // 2. Mode Switching & Joystick Edge Detection
        JoystickDir editor_joy = JOY_CENTER; // Single-event container

        if (joy != last_joy) {
            // Only act on the "Press" (Rising Edge) of the joystick
            if (joy != JOY_CENTER) {
                // Handle Mode Switching (Up/Down)
                if (joy == JOY_DOWN) {
                    if (current_mode >= MODE_GPIO_EXEC) current_mode = MODE_PROGRAM_X;
                    else current_mode++;
                    
                    AppState_SetMode(current_mode);
                    printf("[Mode] Switched to: %s\n", get_mode_name(current_mode));
                    
                    // Load existing equation into editor
                    SharedState st = AppState_GetSnapshot();
                    if (current_mode == MODE_PROGRAM_X) Editor_LoadLine(st.input_x);
                    else if (current_mode == MODE_PROGRAM_Y) Editor_LoadLine(st.input_y);
                    else if (current_mode == MODE_PROGRAM_Z) Editor_LoadLine(st.input_z);
                    else if (current_mode == MODE_PROGRAM_W) Editor_LoadLine(st.input_w);
                }
                else if (joy == JOY_UP) {
                    if (current_mode <= MODE_PROGRAM_X) current_mode = MODE_GPIO_EXEC;
                    else current_mode--;
                    
                    AppState_SetMode(current_mode);
                    printf("[Mode] Switched to: %s\n", get_mode_name(current_mode));

                    SharedState st = AppState_GetSnapshot();
                    if (current_mode == MODE_PROGRAM_X) Editor_LoadLine(st.input_x);
                    else if (current_mode == MODE_PROGRAM_Y) Editor_LoadLine(st.input_y);
                    else if (current_mode == MODE_PROGRAM_Z) Editor_LoadLine(st.input_z);
                    else if (current_mode == MODE_PROGRAM_W) Editor_LoadLine(st.input_w);
                }
                
                // Capture Left/Right for the Editor (Logic Menu)
                if (joy == JOY_LEFT || joy == JOY_RIGHT) {
                    editor_joy = joy;
                }
            }
            last_joy = joy;
        }

        // 3. Execution Logic
        if (current_mode >= MODE_PROGRAM_X && current_mode <= MODE_PROGRAM_W) {
            
            // Pass Rotary and Joystick events to Editor State
            Editor_UpdateState(rot_delta, editor_joy);
            
            // --- SMART MENU PRINTING ---
            const char* current_label = Editor_GetMenuLabel();
            if (strcmp(last_print_buf, current_label) != 0) {
                printf("  [Editor] %s > %s\n", get_mode_name(current_mode), current_label);
                strncpy(last_print_buf, current_label, 63);
            }
            // ---------------------------

            // Handle Button Press (Insert/Delete/Set)
            EditorResult res = Editor_HandleButton(rot_btn);
            
            if (res == EDITOR_RESULT_MODIFIED) {
                // 1. Flash LED for feedback
                flash_active = true;
                led_flash_start = Timer_GetMillis();
                
                // 2. REQUIRED: Print "Queued: <Buffer>"
                printf("Queued: %s\n", Editor_GetLine());
                
                // 3. Update stateless preview (for web/LEDs)
                Process_Stateless("preview", Editor_GetLine());
            }
            else if (res == EDITOR_RESULT_SAVE) {
                const char* final_eq = Editor_GetLine();
                // Save to State
                if (current_mode == MODE_PROGRAM_X) AppState_SetInputX(final_eq);
                else if (current_mode == MODE_PROGRAM_Y) AppState_SetInputY(final_eq);
                else if (current_mode == MODE_PROGRAM_Z) AppState_SetInputZ(final_eq);
                else if (current_mode == MODE_PROGRAM_W) AppState_SetInputW(final_eq);
                
                flash_active = true;
                led_flash_start = Timer_GetMillis() + 200; 
                printf("[Editor] SET %s to: %s\n", get_mode_name(current_mode), final_eq);
            }
            else if (rot_btn != ROT_BTN_NONE && !Editor_IsSyntaxValid() && strcmp(Editor_GetMenuLabel(), "[ SET ]") == 0) {
                 printf("[Editor] Cannot Set: Invalid Syntax\n");
            }
        }
        else if (current_mode == MODE_ROTARY_EXEC) {
            // Cycle A-F and Toggle
            if (rot_delta != 0) {
                run_menu_index += rot_delta;
                while (run_menu_index < 0) run_menu_index += 6;
                while (run_menu_index >= 6) run_menu_index -= 6;
                printf("  [Run Input] Selected: %s\n", RUN_MENU_ITEMS[run_menu_index]);
            }
            
            if (rot_btn == ROT_BTN_CLICK) {
                int bit = run_menu_index; // A=0
                uint8_t mask = AppState_GetInputMask();
                mask ^= (1 << bit);
                AppState_SetInputMask(mask);
                printf("  [Run Input] Toggled %s -> %s\n", RUN_MENU_ITEMS[run_menu_index], (mask >> bit) & 1 ? "ON" : "OFF");
            }
        }

        // 4. Global Updates (LEDs & Outputs)
        if (AppState_IsDirty()) {
            SharedState st = AppState_GetSnapshot();
            
            bool vx = Process_Equation("X", st.input_x, "run");
            bool vy = Process_Equation("Y", st.input_y, "run");
            bool vz = Process_Equation("Z", st.input_z, "run");
            bool vw = Process_Equation("W", st.input_w, "run");
            
            if (st.valid_x != vx || st.valid_y != vy) 
                AppState_SetValidation(vx, vy, vz, vw);

            Send_Combined_Update(st.input_x, st.input_y, st.input_z, st.input_w);
            NetUDP_BroadcastState();

            // Hardware Output
            LogicNode* rx = Parser_ParseString(st.input_x);
            LogicNode* ry = Parser_ParseString(st.input_y);
            LogicNode* rz = Parser_ParseString(st.input_z);
            LogicNode* rw = Parser_ParseString(st.input_w);
            
            bool val_x = AST_Evaluate(rx, st.input_signal_state);
            bool val_y = AST_Evaluate(ry, st.input_signal_state);
            bool val_z = AST_Evaluate(rz, st.input_signal_state);
            bool val_w = AST_Evaluate(rw, st.input_signal_state);
            
            HAL_GPIO_Write(GPIO_OUT_X, val_x);
            HAL_GPIO_Write(GPIO_OUT_Y, val_y);
            HAL_GPIO_Write(GPIO_OUT_Z, val_z);
            HAL_GPIO_Write(GPIO_OUT_W, val_w);
            
            // --- LED OUTPUT MAPPING ---
            if (flash_active) {
                if (Timer_HasElapsed(led_flash_start, 150)) flash_active = false;
                else HAL_LED_SetRGB(255, 255, 0); // Yellow Flash
            } 
            
            if (!flash_active) {
                // Green = X, Red = Y
                HAL_LED_SetRGB(val_y ? 255 : 0, val_x ? 255 : 0, 0);
            }

            AST_Free(rx); AST_Free(ry); AST_Free(rz); AST_Free(rw);

            AppState_ClearDirty();
        }

        usleep(20000); 
    }

    NetUDP_Cleanup();
    HAL_General_Cleanup();
    AppState_Cleanup();
    return 0;
}