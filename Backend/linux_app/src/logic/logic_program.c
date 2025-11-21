/*
 * File: logic_program.c
 * Version: 1.0.0
 * Description:
 * Utilities for direct minterm programming.
 * This module allows configuring the logic engine using raw CSV lists
 * of minterms (e.g., "0, 15, 63") instead of boolean algebra strings.
 *
 * It essentially performs the reverse of the analysis pipeline:
 * Minterms -> Minimization -> SOP Equation -> State Update.
 */

#include "logic_program.h"
#include "app_state.h"
#include "logic_minimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Function: Program_From_Minterms
 * -------------------------------
 * Converts a CSV string of minterms into a minimized boolean equation
 * and updates the application state for the specified target channel.
 *
 * target:      "x", "y", "z", or "w".
 * minterm_csv: Comma-separated string of integers (0-63).
 */
void Program_From_Minterms(const char* target, char* minterm_csv) {
    TruthTable tt;
    tt.count = 0;

    // 1. Parse CSV into Truth Table structure
    char* token = strtok(minterm_csv, ",");
    while (token != NULL && tt.count < MAX_MINTERMS) {
        tt.minterms[tt.count++] = atoi(token);
        token = strtok(NULL, ",");
    }

    // 2. Run Minimization (Recover the equation)
    ImplicantList primes = Minimizer_FindPrimeImplicants(tt);
    char sop_buffer[512];
    Minimizer_PrintSOP(&primes, sop_buffer);

    printf("  [K-Map Input] %s Minterms: [%s] -> SOP: %s\n", 
           target, minterm_csv, sop_buffer);

    // 3. Update Global State with the new equation string
    if (strcmp(target, "x") == 0) AppState_SetInputX(sop_buffer);
    else if (strcmp(target, "y") == 0) AppState_SetInputY(sop_buffer);
    else if (strcmp(target, "z") == 0) AppState_SetInputZ(sop_buffer);
    else if (strcmp(target, "w") == 0) AppState_SetInputW(sop_buffer);
}