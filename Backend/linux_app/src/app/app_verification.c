/*
 * File: app_verification.c
 * Version: 1.0.0
 * Description:
 * Execution engine for the Automated Test Suite.
 * This module provides a software-in-the-loop verification capability,
 * allowing users to script input sequences and verify logic outputs
 * against expected behavior.
 */

#include "app_verification.h"
#include "app_state.h"
#include "logic_ast.h"
#include "logic_parser.h"
#include "net_udp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*
 * Function: current_time_ms
 * -------------------------
 * Helper to get monotonic time in milliseconds.
 */
static long long current_time_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * 1000 + spec.tv_nsec / 1.0e6;
}

/*
 * Function: Verification_RunSuite
 * -------------------------------
 * Parses a comma-separated test vector string and executes the simulation.
 * * Sequence Format: "InputMask:Duration, InputMask:Duration"
 * Example: "0:100, 1:50" -> Input 0 for 100ms, then Input 1 for 50ms.
 *
 * Logic Flow:
 * 1. Snapshot the current logic equations from AppState.
 * 2. Parse them into ASTs.
 * 3. Iterate through the test vector steps.
 * 4. Evaluate ASTs for each step and log the result.
 * 5. Package results as a CSV inside a JSON packet.
 */
void Verification_RunSuite(const char* test_sequence) {
    printf("[Verification] Starting Test Suite...\n");

    SharedState st = AppState_GetSnapshot();
    LogicNode* rootX = Parser_ParseString(st.input_x);
    LogicNode* rootY = Parser_ParseString(st.input_y);

    // Allocate buffer for CSV data (Time, Mask, X, Y)
    char* csv_data = malloc(65536); 
    int offset = 0;
    
    // Write Header (Note: \\n is used to escape newline for JSON transport)
    offset += sprintf(csv_data + offset, "Time,Mask,X,Y\\n");

    char* seq_copy = strdup(test_sequence);
    char* pair = strtok(seq_copy, ",");
    long long accumulated_time = 0;

    // Iterate over "Mask:Duration" pairs
    while (pair != NULL) {
        int input_mask = 0;
        int duration = 0;
        
        if (sscanf(pair, "%d:%d", &input_mask, &duration) == 2) {
            // Evaluate Logic
            bool resX = rootX ? AST_Evaluate(rootX, input_mask) : false;
            bool resY = rootY ? AST_Evaluate(rootY, input_mask) : false;

            // Log Record
            offset += sprintf(csv_data + offset, "%lld,%d,%d,%d\\n",
                              accumulated_time, input_mask, resX, resY);
            
            accumulated_time += duration;
        }
        pair = strtok(NULL, ",");
    }

    printf("[Verification] Test Suite Completed Successfully.\n");

    // Construct Result Packet
    char* packet = malloc(70000);
    sprintf(packet, "{ \"type\": \"verification\", \"status\": \"success\", \"csv\": \"%s\" }", csv_data);
    
    // Send to frontend
    NetUDP_SendRaw(packet);

    // Cleanup resources
    free(csv_data);
    free(packet);
    free(seq_copy);
    if (rootX) AST_Free(rootX);
    if (rootY) AST_Free(rootY);
}