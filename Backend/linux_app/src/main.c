/*
 * File: main.c
 * Version: 1.1.1
 * Description:
 * The main entry point and Event Loop.
 * Orchestrates Hardware Inputs -> Application Logic -> Hardware Outputs.
 */

#include <stdio.h>
#include <unistd.h>

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

// --- Added these missing includes for GPIO Logic Evaluation ---
#include "logic_parser.h"
#include "logic_ast.h" 

// Helper to map Mode Enum to String
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

int main() {
    // 1. Init
    AppState_Init();
    Editor_Init();
    HAL_General_Init();
    NetUDP_Init();

    printf(C_B_GREEN "=== LOGIC SIM ENGINE STARTED ===" C_RESET "\n");

    // State Tracking
    SystemMode current_mode = MODE_PROGRAM_X;
    JoystickDir last_joy = JOY_CENTER;
    long long led_flash_start = 0;
    bool flash_active = false;

    // 2. Main Loop (20ms = 50Hz)
    while (1) {
        if (NetUDP_ExitRequested()) break;

        // --- A. INPUT POLLING ---
        JoystickDir joy = HAL_Joystick_GetDir();
        int rot_delta = HAL_Rotary_GetCount();
        RotaryButtonState rot_btn = HAL_Rotary_GetButtonEvent();

        // --- B. MODE SWITCHING (Joystick UP/DOWN) ---
        // Simple debouncing: only act if direction changed
        if (joy != last_joy) {
            if (joy == JOY_DOWN) {
                current_mode++;
                if (current_mode > MODE_GPIO_EXEC) current_mode = MODE_PROGRAM_X;
                AppState_SetMode(current_mode);
                printf("[Mode] Switched to: %s\n", get_mode_name(current_mode));
                
                // Load Editor buffer if entering Program Mode
                SharedState st = AppState_GetSnapshot();
                if (current_mode == MODE_PROGRAM_X) Editor_LoadLine(st.input_x);
                if (current_mode == MODE_PROGRAM_Y) Editor_LoadLine(st.input_y);
                if (current_mode == MODE_PROGRAM_Z) Editor_LoadLine(st.input_z);
                if (current_mode == MODE_PROGRAM_W) Editor_LoadLine(st.input_w);
            }
            last_joy = joy;
        }

        // --- C. MODE EXECUTION ---
        if (current_mode >= MODE_PROGRAM_X && current_mode <= MODE_PROGRAM_W) {
            // === PROGRAMMING MODE ===
            
            // 1. Update Editor State
            Editor_UpdateState(rot_delta, joy);

            // 2. Handle Button Clicks
            EditorResult res = Editor_HandleButton(rot_btn);
            
            if (res == EDITOR_RESULT_MODIFIED) {
                flash_active = true;
                led_flash_start = Timer_GetMillis();
                // Preview changes Statelessly
                Process_Stateless("preview", Editor_GetLine());
            }
            else if (res == EDITOR_RESULT_SAVE) {
                // Commit to App State
                const char* final_eq = Editor_GetLine();
                if (current_mode == MODE_PROGRAM_X) AppState_SetInputX(final_eq);
                else if (current_mode == MODE_PROGRAM_Y) AppState_SetInputY(final_eq);
                else if (current_mode == MODE_PROGRAM_Z) AppState_SetInputZ(final_eq);
                else if (current_mode == MODE_PROGRAM_W) AppState_SetInputW(final_eq);
                
                // Visual Confirmation (Long Flash)
                flash_active = true;
                led_flash_start = Timer_GetMillis() + 200; 
                printf("[Editor] Saved: %s\n", final_eq);
            }

            // 3. LED Feedback (Green = Valid Syntax)
            bool valid = Editor_IsSyntaxValid();
            
            // Red LED Flash logic
            bool red_on = false;
            if (flash_active) {
                if (Timer_HasElapsed(led_flash_start, 100)) flash_active = false;
                else red_on = true;
            }
            
            HAL_LED_SetRGB(red_on ? 255 : 0, valid ? 255 : 0, 0);
        }
        else if (current_mode == MODE_ROTARY_EXEC) {
            // === ROTARY RUN MODE ===
            // Requirement #8: Toggle Inputs A-F
            // Map Menu Selection A-F to Bitmask 0-5
            
            Editor_UpdateState(rot_delta, JOY_CENTER); // Just scroll menu
            
            if (rot_btn == ROT_BTN_CLICK) {
                const char* label = Editor_GetMenuLabel();
                // Check if label is "[ A ]" ... "[ F ]"
                // Simple hack: check 3rd char
                char c = label[2]; 
                if (c >= 'A' && c <= 'F') {
                    int bit = c - 'A';
                    uint8_t mask = AppState_GetInputMask();
                    mask ^= (1 << bit); // Toggle
                    AppState_SetInputMask(mask);
                }
            }
            HAL_LED_SetRGB(0, 0, 255); // Blue for RUN
        }
        else if (current_mode == MODE_GPIO_EXEC) {
            // === GPIO RUN MODE ===
            // Requirement #9: Read Physical Pins
            uint8_t mask = 0;
            if (HAL_GPIO_Read(GPIO_IN_A)) mask |= (1 << 0);
            if (HAL_GPIO_Read(GPIO_IN_B)) mask |= (1 << 1);
            if (HAL_GPIO_Read(GPIO_IN_C)) mask |= (1 << 2);
            if (HAL_GPIO_Read(GPIO_IN_D)) mask |= (1 << 3);
            if (HAL_GPIO_Read(GPIO_IN_E)) mask |= (1 << 4);
            if (HAL_GPIO_Read(GPIO_IN_F)) mask |= (1 << 5);
            
            AppState_SetInputMask(mask);
            HAL_LED_SetRGB(0, 255, 0); // Green for GPIO RUN
        }

        // --- D. GLOBAL UPDATE ---
        if (AppState_IsDirty()) {
            SharedState st = AppState_GetSnapshot();
            
            // Process Logic
            bool vx = Process_Equation("X", st.input_x, "run");
            bool vy = Process_Equation("Y", st.input_y, "run");
            bool vz = Process_Equation("Z", st.input_z, "run");
            bool vw = Process_Equation("W", st.input_w, "run");
            
            if (st.valid_x != vx || st.valid_y != vy) 
                AppState_SetValidation(vx, vy, vz, vw);

            // Send Updates
            Send_Combined_Update(st.input_x, st.input_y, st.input_z, st.input_w);
            NetUDP_BroadcastState();

            // Update Physical Outputs (Requirement #9)
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

            AST_Free(rx); AST_Free(ry); AST_Free(rz); AST_Free(rw);

            AppState_ClearDirty();
        }

        usleep(20000); // 20ms loop
    }

    // Cleanup
    NetUDP_Cleanup();
    HAL_General_Cleanup();
    AppState_Cleanup();
    return 0;
}