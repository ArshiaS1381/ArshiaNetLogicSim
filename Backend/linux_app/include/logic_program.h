/*
 * File: logic_program.h
 * Version: 1.0.0
 * Description:
 * Utilities for "programming" the logic server via direct minterm lists.
 * This allows setting the behavior of an output by specifying exactly
 * which input combinations should result in a High output, bypassing
 * algebraic parsing.
 */

#ifndef LOGIC_PROGRAM_H
#define LOGIC_PROGRAM_H

/*
 * Function: Program_From_Minterms
 * -------------------------------
 * Configures a specific target output (X, Y, Z, or W) based on a CSV list of minterms.
 *
 * target:      The identifier of the output to program (e.g., "X").
 * minterm_csv: A string of comma-separated integers (e.g., "0, 2, 5, 7").
 * Each integer represents an input state (0-63) where output is High.
 */
void Program_From_Minterms(const char* target, char* minterm_csv);

#endif