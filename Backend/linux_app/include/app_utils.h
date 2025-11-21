/*
 * File: app_utils.h
 * Version: 1.0.0
 * Description:
 * Provides high-level utility functions that bridge the gap between
 * raw logic parsing and the application state.
 *
 * This module handles the orchestration of compiling equations,
 * updating the global state, and triggering network updates.
 */

#ifndef APP_UTILS_H
#define APP_UTILS_H

#include <stdbool.h>

/*
 * Function: Process_Equation
 * --------------------------
 * Orchestrates the full compilation pipeline for a single equation:
 * 1. Parses the expression string into an AST.
 * 2. If valid, minimizes the logic (using Quine-McCluskey).
 * 3. Updates the application state with the result.
 * 4. Sends the new configuration to the UDP clients.
 *
 * label:      Identifier for the channel (e.g., "X").
 * expression: The boolean string to compile.
 * mode:       Context string used for logging or network packets.
 *
 * returns:    true if the equation was successfully parsed and processed.
 */
bool Process_Equation(const char* label, const char* expression, const char* mode);

/*
 * Function: Send_Combined_Update
 * ------------------------------
 * Aggregates the current state of all four channels (X, Y, Z, W)
 * and broadcasts a unified JSON update to all connected clients.
 * This ensures the front-end view remains synchronized.
 *
 * in_x, in_y...: The current input strings for each channel.
 */
void Send_Combined_Update(const char* in_x, const char* in_y, const char* in_z, const char* in_w);

/*
 * Function: Process_Stateless
 * ---------------------------
 * Compiles and evaluates an equation without saving it to the
 * persistent application state. Useful for "Check Syntax" features
 * or temporary calculations.
 *
 * label:      Identifier for the operation.
 * expression: The boolean string to evaluate.
 */
void Process_Stateless(const char* label, const char* expression);

#endif