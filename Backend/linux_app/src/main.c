/*
 * File: main.c
 * Version: 1.0.0
 * Description:
 * The main entry point for the Digital Logic Server.
 * This program initializes the system state, networking, and hardware abstractions,
 * then enters an infinite loop to process logic updates.
 *
 * Architecture:
 * - The main loop monitors the 'dirty' flag in AppState.
 * - When changes are detected (from UDP commands or internal events), it
 * re-compiles the logic equations and broadcasts the results.
 * - This separation allows the UDP thread to receive commands asynchronously
 * without blocking the core logic processing.
 */

#include <stdio.h>
#include <unistd.h>

#include "app_state.h"
#include "net_udp.h"
#include "app_utils.h"
#include "utils_colors.h"
#include "hal_general.h" // Added for hardware init

int main() {
    // Startup Banner
    printf(C_B_GREEN "\n========================================\n" C_RESET);
    printf(C_B_WHITE "   LOGIC SIM CAPSTONE - ENGINE START    \n" C_RESET);
    printf(C_B_GREEN "========================================\n" C_RESET);

    // 1. Initialization Phase
    // Initialize thread-safe state storage
    AppState_Init();
    
    // Initialize Hardware Abstraction Layer (GPIO, I2C, IPC)
    HAL_General_Init();

    // Start the UDP Networking thread
    NetUDP_Init();

    printf("[Main] System Ready. Waiting for input...\n");

    // 2. Main Execution Loop
    // Runs indefinitely, polling for state changes and executing logic updates.
    while (1) {
        // Check if the application state needs processing
        if (AppState_IsDirty()) {
            SharedState st = AppState_GetSnapshot();
            
            printf("[Main] State change detected. Reprocessing...\n");

            // Process all four logic channels independently
            // The 'program' mode tag indicates this is a persistent update
            bool vx = Process_Equation("X", st.input_x, "program");
            bool vy = Process_Equation("Y", st.input_y, "program");
            bool vz = Process_Equation("Z", st.input_z, "program");
            bool vw = Process_Equation("W", st.input_w, "program");

            // Update validation flags if the compilation status changed
            if (st.valid_x != vx || st.valid_y != vy || st.valid_z != vz || st.valid_w != vw) {
                AppState_SetValidation(vx, vy, vz, vw);
            }

            // Broadcast the new system state to the UI (Node.js)
            Send_Combined_Update(st.input_x, st.input_y, st.input_z, st.input_w);
            NetUDP_BroadcastState();

            // Reset the dirty flag to prevent unnecessary re-processing
            AppState_ClearDirty();
            printf(C_B_GREEN "✔ Global Update Complete." C_RESET "\n");
        }

        // Sleep to yield CPU time (100ms poll rate)
        // This prevents the loop from consuming 100% of a core while idle.
        usleep(100000); 
    }

    // 3. Cleanup Phase
    // (Unreachable code in this loop structure, but good practice for OS signals)
    NetUDP_Cleanup();
    AppState_Cleanup();
    HAL_General_Cleanup();
    return 0;
}