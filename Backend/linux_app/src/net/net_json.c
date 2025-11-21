/*
 * File: net_json.c
 * Version: 1.0.0
 * Description:
 * Lightweight JSON serialization helper.
 * Provides simple string formatting functions to construct valid JSON objects
 * directly from C structures.
 *
 * This avoids the overhead of a full JSON library (like cJSON) since the
 * output format is strictly controlled and known at compile time.
 */

#include "net_json.h"
#include <stdio.h>

/*
 * Function: JSON_SerializeState
 * -----------------------------
 * Serializes the SharedState struct into a JSON object.
 * Boolean values are converted to "true"/"false" literals.
 */
void JSON_SerializeState(const SharedState* state, char* buffer, int len) {
    snprintf(buffer, len,
        "{"
        "\"mode\": %d,"
        "\"input_x\": \"%s\","
        "\"input_y\": \"%s\","
        "\"input_z\": \"%s\","
        "\"input_w\": \"%s\","
        "\"valid_x\": %s,"
        "\"valid_y\": %s,"
        "\"valid_z\": %s,"
        "\"valid_w\": %s"
        "}",
        state->mode,
        state->input_x,
        state->input_y,
        state->input_z,
        state->input_w,
        state->valid_x ? "true" : "false",
        state->valid_y ? "true" : "false",
        state->valid_z ? "true" : "false",
        state->valid_w ? "true" : "false"
    );
}

/*
 * Function: JSON_SerializeMessage
 * -------------------------------
 * Wraps a simple text string in a JSON object.
 */
void JSON_SerializeMessage(const char* msg, char* buffer, int len) {
    snprintf(buffer, len, "{ \"message\": \"%s\" }", msg);
}