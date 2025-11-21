/*
 * File: net_json.h
 * Version: 1.0.0
 * Description:
 * Helper module for serializing application state into JSON format.
 * Used primarily for communicating status updates to connected UDP clients
 * or web interfaces.
 */

#ifndef NET_JSON_H
#define NET_JSON_H

#include "app_state.h"

/*
 * Function: JSON_SerializeState
 * -----------------------------
 * Converts the entire SharedState structure into a JSON string.
 * This includes current input values, output calculations, and mode flags.
 *
 * state:  Pointer to the global application state.
 * buffer: Output buffer for the JSON string.
 * len:    Maximum length of the buffer.
 */
void JSON_SerializeState(const SharedState* state, char* buffer, int len);

/*
 * Function: JSON_SerializeMessage
 * -------------------------------
 * Creates a simple wrapper JSON object for sending text alerts.
 * Format: { "message": "your text here" }
 *
 * msg:    The raw text message to encapsulate.
 * buffer: Output buffer for the JSON string.
 * len:    Maximum length of the buffer.
 */
void JSON_SerializeMessage(const char* msg, char* buffer, int len);

#endif