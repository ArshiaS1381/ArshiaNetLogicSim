/*
 * File: app_verification.h
 * Version: 1.0.0
 * Description:
 * Manages the verification and testing suite for the digital logic server.
 * This module is responsible for parsing test vectors (formatted as text strings),
 * simulating the current logic configuration against those vectors, and
 * recording the results (pass/fail and timing).
 *
 * It is designed to allow users to script inputs (e.g., "Set input 5, wait 100ms")
 * to verify circuit behavior before interfacing with physical hardware.
 */

#ifndef APP_VERIFICATION_H
#define APP_VERIFICATION_H

#include <stdbool.h>

/*
 * Constant: MAX_TEST_STEPS
 * ------------------------
 * Defines the maximum number of distinct steps allowed in a single test script.
 * A "step" consists of setting specific inputs and holding them for a duration.
 */
#define MAX_TEST_STEPS 100

/*
 * Struct: TestResult
 * ------------------
 * Represents a single data point captured during a simulation run.
 *
 * timestamp:  System time (in milliseconds) when this sample was taken.
 * input_mask: An integer bitmask representing the state of inputs A-F.
 * (e.g., bit 0 = A, bit 1 = B, etc.)
 * out_x:      The Logic Level (High/Low) of output X at this timestamp.
 * out_y:      The Logic Level (High/Low) of output Y at this timestamp.
 */
typedef struct {
    long long timestamp;
    int input_mask;
    bool out_x;
    bool out_y;
} TestResult;

/*
 * Function: Verification_RunSuite
 * -------------------------------
 * Parses a formatted command string and executes a timed simulation sequence.
 *
 * This function decodes the string, applies the inputs to the AST, waits for
 * the specified duration, and records the output state.
 *
 * test_sequence: A string defining the test pattern.
 * Format: "InputMask:DurationMS, InputMask:DurationMS, ..."
 * Example: "0:100, 1:100, 3:50"
 * - Sets inputs to 0 (All Low) for 100ms.
 * - Sets inputs to 1 (A High) for 100ms.
 * - Sets inputs to 3 (A and B High) for 50ms.
 *
 * returns: void (Results are typically stored in a global state or broadcast via UDP).
 */
void Verification_RunSuite(const char* test_sequence);

#endif