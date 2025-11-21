/*
 * File: app_state.h
 * Version: 1.0.0
 * Description:
 * Defines the shared application state structure and thread synchronization mechanisms.
 * This module acts as the central data repository for the application, holding
 * the current operating mode, logic input strings, and validation status.
 *
 * Since multiple threads (UDP listener, Logic Analyzer, User Interface) may
 * access this data simultaneously, a mutex is managed internally to prevent
 * race conditions.
 */

#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>
#include <pthread.h>

/*
 * Enum: SystemMode
 * ----------------
 * Enumerates the distinct operational states of the application.
 *
 * MODE_PROGRAM_X/Y/Z/W: The user is currently typing/editing the logic equation
 * for a specific output channel.
 * MODE_RUN:             The system is actively simulating the logic equations.
 * MODE_TESTING:         The system is running automated verification scripts.
 */
typedef enum {
    MODE_PROGRAM_X = 0,
    MODE_PROGRAM_Y,
    MODE_PROGRAM_Z,
    MODE_PROGRAM_W,
    MODE_RUN,
    MODE_TESTING
} SystemMode;

/*
 * Struct: SharedState
 * -------------------
 * The global state object shared across threads.
 *
 * mode:     The current active mode of the system.
 * is_dirty: A generic flag used to signal that the state has been modified
 * and views (like the UI or Network) should refresh.
 *
 * input_x/y/z/w: String buffers holding the user's boolean expressions
 * (e.g., "A * B") for each output channel.
 *
 * valid_x/y/z/w: Booleans indicating if the current string in the input buffer
 * successfully compiles into a valid logic tree.
 */
typedef struct {
    SystemMode mode;
    bool is_dirty;

    char input_x[256];
    char input_y[256];
    char input_z[256];
    char input_w[256];
    
    bool valid_x;
    bool valid_y;
    bool valid_z;
    bool valid_w;
} SharedState;

/*
 * Function: AppState_Init
 * -----------------------
 * Allocates resources and initializes the mutex for state management.
 * Sets default values for the application state (typically MODE_RUN).
 */
void AppState_Init(void);

/*
 * Function: AppState_Cleanup
 * --------------------------
 * Destroys the mutex and releases any resources held by the state module.
 * Call this before application exit.
 */
void AppState_Cleanup(void);

/*
 * Function: AppState_GetSnapshot
 * ------------------------------
 * Returns a copy of the current shared state in a thread-safe manner.
 * Use this when you need to read the state without blocking other threads
 * for an extended period.
 */
SharedState AppState_GetSnapshot(void);

/*
 * Function: AppState_SetInputX (and Y, Z, W)
 * ------------------------------------------
 * Updates the logic equation string for the specified channel.
 * These functions automatically acquire the mutex to ensure data integrity.
 *
 * str: The new equation string (null-terminated).
 */
void AppState_SetInputX(const char* str);
void AppState_SetInputY(const char* str);
void AppState_SetInputZ(const char* str);
void AppState_SetInputW(const char* str);

/*
 * Function: AppState_SetValidation
 * --------------------------------
 * Updates the validity flags for all four channels simultaneously.
 *
 * vx, vy, vz, vw: True if the corresponding channel's equation is valid.
 */
void AppState_SetValidation(bool vx, bool vy, bool vz, bool vw);

/*
 * Function: AppState_IsDirty
 * --------------------------
 * Checks if the global state has changed since the last clear.
 * Useful for polling loops to determine if a redraw is necessary.
 *
 * returns: true if modifications have occurred.
 */
bool AppState_IsDirty(void);

/*
 * Function: AppState_ClearDirty
 * -----------------------------
 * Resets the dirty flag to false.
 * Should be called by the main loop after it has finished processing updates.
 */
void AppState_ClearDirty(void);

/*
 * Function: AppState_Touch
 * ------------------------
 * Manually sets the dirty flag to true.
 * Used to force a system-wide refresh even if data hasn't explicitly changed.
 */
void AppState_Touch(void);

#endif