/*
 * File: logic_minimizer.h
 * Version: 1.0.0
 * Description:
 * Implements the Quine-McCluskey algorithm for logic minimization.
 * This module is responsible for taking a raw Logic AST, converting it
 * into a truth table, and then reducing it to its simplest Sum-of-Products (SOP)
 * or Product-of-Sums (POS) form.
 *
 * This is essential for optimizing the logic before sending it to hardware
 * or displaying the simplified equation to the user.
 */

#ifndef LOGIC_MINIMIZER_H
#define LOGIC_MINIMIZER_H

#include "logic_ast.h"
#include <stdbool.h>

#define MAX_MINTERMS 64
#define MAX_VARS 6

/*
 * Struct: Implicant
 * -----------------
 * Represents a single product term in the minimization process.
 * Used to track groups of minterms that can be combined.
 *
 * value:        The binary value of the term.
 * mask:         Bitmask indicating which variables are "Don't Cares" (dashes).
 * If a bit in 'mask' is 1, that variable is eliminated.
 * used:         Algorithm flag; true if this term has been combined into a larger group.
 * is_essential: True if this term is a Prime Implicant that covers a unique minterm.
 */
typedef struct {
    int value;
    int mask;
    bool used;
    bool is_essential; 
} Implicant;

/*
 * Struct: ImplicantList
 * ---------------------
 * A dynamic list of Implicants used during the Quine-McCluskey passes.
 */
typedef struct {
    Implicant terms[512];
    int count;
} ImplicantList;

/*
 * Struct: TruthTable
 * ------------------
 * Represents the raw truth table of a logic function.
 *
 * minterms: Array containing the indices (0-63) where the function outputs 1.
 * count:    The total number of minterms found.
 */
typedef struct {
    int minterms[MAX_MINTERMS];
    int count;
} TruthTable;

/*
 * Function: Minimizer_GenerateTruthTable
 * --------------------------------------
 * Brute-force evaluates the AST against all possible input combinations
 * to generate a complete truth table.
 *
 * root:    Pointer to the logic tree.
 *
 * returns: A TruthTable struct containing all inputs that result in a '1'.
 */
TruthTable Minimizer_GenerateTruthTable(LogicNode* root);

/*
 * Function: Minimizer_FindPrimeImplicants
 * ---------------------------------------
 * Executes the core Quine-McCluskey algorithm.
 * It iteratively combines minterms that differ by one bit until no further
 * combinations are possible.
 *
 * tt:      The input Truth Table.
 *
 * returns: A list of all Prime Implicants found.
 */
ImplicantList Minimizer_FindPrimeImplicants(TruthTable tt);

/*
 * Function: Minimizer_PrintSOP
 * ----------------------------
 * Formats the Prime Implicants into a readable "Sum of Products" string.
 * Example: "A B' + C D"
 *
 * list:   Pointer to the list of Prime Implicants.
 * buffer: Character buffer to write the string into.
 */
void Minimizer_PrintSOP(ImplicantList* list, char* buffer);

/*
 * Function: Minimizer_PrintPOS
 * ----------------------------
 * Formats the implication list into a "Product of Sums" string.
 * Example: "(A + B) * (C' + D)"
 * Note: This requires calculating the maxterms first.
 *
 * list:   Pointer to the list of implicants derived from Maxterms.
 * buffer: Character buffer to write the string into.
 */
void Minimizer_PrintPOS(ImplicantList* list, char* buffer);

/*
 * Function: Minimizer_GetMaxterms
 * -------------------------------
 * Inverts the logic of the AST to find the Maxterms (inputs where result is 0).
 * Used for generating Product of Sums (POS) expressions.
 *
 * root:    Pointer to the logic tree.
 *
 * returns: A TruthTable struct containing all Maxterms.
 */
TruthTable Minimizer_GetMaxterms(LogicNode* root);

#endif