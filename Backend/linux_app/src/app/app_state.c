/*
 * File: app_state.c
 * Version: 1.0.0
 * Description:
 * Implements the central data store for the application.
 * This module manages the 'SharedState' structure, which acts as the
 * Single Source of Truth (SSOT) for the system.
 *
 * Key Responsibilities:
 * 1. Storing the current logic equations (X, Y, Z, W).
 * 2. Managing the system operational mode.
 * 3. Providing thread-safe access via mutex locking to prevent race conditions
 * between the UDP networking thread and the main execution loop.
 */

#include "app_state.h"
#include <string.h>
#include <stdio.h>

// Global instance of the application state
static SharedState global_state;

// Mutex to protect concurrent access to global_state
static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Function: AppState_Init
 * -----------------------
 * Zeroes out the memory for the state structure and sets default values.
 * Must be called before any other AppState function.
 */
void AppState_Init(void) {
    pthread_mutex_lock(&state_mutex);
    
    memset(&global_state, 0, sizeof(SharedState));
    global_state.mode = MODE_PROGRAM_X; // Default to programming X
    global_state.is_dirty = true;       // Force initial refresh
    
    // Initialize buffers to empty strings to prevent garbage reads
    global_state.input_x[0] = '\0';
    global_state.input_y[0] = '\0';
    global_state.input_z[0] = '\0';
    global_state.input_w[0] = '\0';

    pthread_mutex_unlock(&state_mutex);
    printf("[App State] Initialized (4-Channel)\n");
}

/*
 * Function: AppState_Cleanup
 * --------------------------
 * Destroys the mutex. Used during system shutdown.
 */
void AppState_Cleanup(void) {
    pthread_mutex_destroy(&state_mutex);
}

/*
 * Function: AppState_GetSnapshot
 * ------------------------------
 * Returns a complete copy of the state struct.
 * This allows a consumer (like the UI renderer) to read consistent data
 * without holding the lock for the duration of the rendering process.
 */
SharedState AppState_GetSnapshot(void) {
    SharedState snapshot;
    pthread_mutex_lock(&state_mutex);
    snapshot = global_state;
    pthread_mutex_unlock(&state_mutex);
    return snapshot;
}

/*
 * Function: AppState_SetMode
 * --------------------------
 * Transitions the system execution mode.
 * Sets the dirty flag to ensure the UI updates to reflect the change.
 */
void AppState_SetMode(SystemMode new_mode) {
    pthread_mutex_lock(&state_mutex);
    if (global_state.mode != new_mode) {
        global_state.mode = new_mode;
        global_state.is_dirty = true;
    }
    pthread_mutex_unlock(&state_mutex);
}

/*
 * Function: AppState_SetInputX
 * ----------------------------
 * Updates the logic equation for Channel X.
 * Uses strncpy to prevent buffer overflow.
 */
void AppState_SetInputX(const char* str) {
    pthread_mutex_lock(&state_mutex);
    strncpy(global_state.input_x, str, 255);
    global_state.input_x[255] = '\0'; // Ensure null-termination
    global_state.is_dirty = true;
    pthread_mutex_unlock(&state_mutex);
}

/*
 * Function: AppState_SetInputY
 * ----------------------------
 * Updates the logic equation for Channel Y.
 */
void AppState_SetInputY(const char* str) {
    pthread_mutex_lock(&state_mutex);
    strncpy(global_state.input_y, str, 255);
    global_state.input_y[255] = '\0';
    global_state.is_dirty = true;
    pthread_mutex_unlock(&state_mutex);
}

/*
 * Function: AppState_SetInputZ
 * ----------------------------
 * Updates the logic equation for Channel Z.
 */
void AppState_SetInputZ(const char* str) {
    pthread_mutex_lock(&state_mutex);
    strncpy(global_state.input_z, str, 255);
    global_state.input_z[255] = '\0';
    global_state.is_dirty = true;
    pthread_mutex_unlock(&state_mutex);
}

/*
 * Function: AppState_SetInputW
 * ----------------------------
 * Updates the logic equation for Channel W.
 */
void AppState_SetInputW(const char* str) {
    pthread_mutex_lock(&state_mutex);
    strncpy(global_state.input_w, str, 255);
    global_state.input_w[255] = '\0';
    global_state.is_dirty = true;
    pthread_mutex_unlock(&state_mutex);
}

/*
 * Function: AppState_SetValidation
 * --------------------------------
 * Atomically updates the validity status of all four channels.
 * This ensures that we don't display a "Valid" status for a channel
 * that is out of sync with its neighbor.
 */
void AppState_SetValidation(bool vx, bool vy, bool vz, bool vw) {
    pthread_mutex_lock(&state_mutex);
    global_state.valid_x = vx;
    global_state.valid_y = vy;
    global_state.valid_z = vz;
    global_state.valid_w = vw;
    global_state.is_dirty = true;
    pthread_mutex_unlock(&state_mutex);
}

/*
 * Function: AppState_IsDirty
 * --------------------------
 * Thread-safe check for the dirty flag.
 */
bool AppState_IsDirty(void) {
    bool d;
    pthread_mutex_lock(&state_mutex);
    d = global_state.is_dirty;
    pthread_mutex_unlock(&state_mutex);
    return d;
}

/*
 * Function: AppState_ClearDirty
 * -----------------------------
 * Resets the dirty flag. Should be called after the UI has refreshed.
 */
void AppState_ClearDirty(void) {
    pthread_mutex_lock(&state_mutex);
    global_state.is_dirty = false;
    pthread_mutex_unlock(&state_mutex);
}

/*
 * Function: AppState_Touch
 * ------------------------
 * Forces a refresh by setting the dirty flag.
 */
void AppState_Touch(void) {
    pthread_mutex_lock(&state_mutex);
    global_state.is_dirty = true;
    pthread_mutex_unlock(&state_mutex);
}