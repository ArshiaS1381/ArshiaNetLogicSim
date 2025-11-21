/*
 * File: logic_minimizer.c
 * Version: 1.0.0
 * Description:
 * The Quine-McCluskey Optimization Engine.
 * This module is the mathematical core of the application. It reduces
 * complex logic trees into their simplest possible Sum-of-Products (SOP)
 * or Product-of-Sums (POS) representations.
 */

#include "logic_minimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Function: Minimizer_GenerateTruthTable
 * --------------------------------------
 * Brute-force generation of the truth table by evaluating the AST
 * for all 64 possible input combinations (assuming 6 vars max).
 */
TruthTable Minimizer_GenerateTruthTable(LogicNode* root) {
    TruthTable table;
    table.count = 0;
    if (!root) return table;

    for (int i = 0; i < 64; i++) {
        if (AST_Evaluate(root, i)) {
            table.minterms[table.count++] = i;
        }
    }
    return table;
}

// --- Quine-McCluskey Helper Functions ---

/*
 * Function: can_combine
 * ---------------------
 * Determines if two implicants can be merged.
 * Two terms merge if they have the same "Dash" mask and differ
 * by exactly one bit in their values.
 */
bool can_combine(Implicant a, Implicant b, Implicant* res) {
    if (a.mask != b.mask) return false; // Structure must match
    
    int diff = a.value ^ b.value;
    // Check if diff is a power of 2 (exactly one bit set)
    if (diff && !(diff & (diff - 1))) {
        res->value = a.value & ~diff; // Normalize value
        res->mask = a.mask | diff;    // Add the new dash
        res->used = false;
        res->is_essential = false;
        return true;
    }
    return false;
}

/*
 * Function: term_exists
 * ---------------------
 * Deduplication helper to prevent adding the same Prime Implicant twice.
 */
bool term_exists(ImplicantList* list, Implicant t) {
    for(int i=0; i<list->count; i++) {
        if (list->terms[i].value == t.value && list->terms[i].mask == t.mask)
            return true;
    }
    return false;
}

/*
 * Function: Minimizer_FindPrimeImplicants
 * ---------------------------------------
 * The core iterative reduction loop.
 * Continues combining terms until no further reductions are possible.
 */
ImplicantList Minimizer_FindPrimeImplicants(TruthTable tt) {
    ImplicantList current_pass;
    current_pass.count = 0;
    
    // Initialize: Convert minterms to Implicants
    for(int i=0; i<tt.count; i++) {
        current_pass.terms[i].value = tt.minterms[i];
        current_pass.terms[i].mask = 0;
        current_pass.terms[i].used = false;
        current_pass.terms[i].is_essential = false;
        current_pass.count++;
    }

    ImplicantList primes;
    primes.count = 0;

    bool changed = true;
    while (changed) {
        changed = false;
        ImplicantList next_pass;
        next_pass.count = 0;
        
        for(int i=0; i<current_pass.count; i++) current_pass.terms[i].used = false;

        // O(N^2) Comparison
        for (int i = 0; i < current_pass.count; i++) {
            for (int j = i + 1; j < current_pass.count; j++) {
                Implicant combined;
                if (can_combine(current_pass.terms[i], current_pass.terms[j], &combined)) {
                    current_pass.terms[i].used = true;
                    current_pass.terms[j].used = true;
                    
                    if (!term_exists(&next_pass, combined)) {
                        next_pass.terms[next_pass.count++] = combined;
                        changed = true;
                    }
                }
            }
        }

        // Collect terms that couldn't be combined (Prime Implicants)
        for (int i = 0; i < current_pass.count; i++) {
            if (!current_pass.terms[i].used) {
                if (!term_exists(&primes, current_pass.terms[i])) {
                    primes.terms[primes.count++] = current_pass.terms[i];
                }
            }
        }

        if (changed) {
            current_pass = next_pass;
        }
    }
    
    return primes;
}

/*
 * Function: Minimizer_PrintSOP
 * ----------------------------
 * Formatting helper. Converts internal Implicant structures into
 * human-readable Boolean Algebra strings (Sum of Products).
 */
void Minimizer_PrintSOP(ImplicantList* list, char* buffer) {
    if (list->count == 0) {
        sprintf(buffer, "0 (False)");
        return;
    }

    // Check for "Always True" (Mask 63 means all 6 vars are dashes)
    if (list->terms[0].mask == 63) {
        sprintf(buffer, "1 (True)");
        return;
    }

    buffer[0] = '\0';
    
    for (int i = 0; i < list->count; i++) {
        if (i > 0) strcat(buffer, " + ");
        
        Implicant t = list->terms[i];
        bool first_var = true;
        
        for (int bit = 0; bit < MAX_VARS; bit++) {
            // If bit is NOT a dash...
            if (!((t.mask >> bit) & 1)) {
                char var = 'A' + bit;
                bool is_true = (t.value >> bit) & 1;
                
                char term[4];
                if (is_true) sprintf(term, "%c", var);
                else         sprintf(term, "%c'", var);
                
                strcat(buffer, term);
                first_var = false;
            }
        }
    }
}

/*
 * Function: Minimizer_PrintPOS
 * ----------------------------
 * Formatting helper. Converts Maxterms into Product of Sums.
 * Inverts the standard logic: 0 becomes the literal, 1 becomes Not-Literal.
 */
void Minimizer_PrintPOS(ImplicantList* list, char* buffer) {
    if (list->count == 0) {
        sprintf(buffer, "1 (True)"); // Empty maxterm list means never false
        return;
    }
    if (list->terms[0].mask == 63) {
        sprintf(buffer, "0 (False)");
        return;
    }

    buffer[0] = '\0'; 
    
    for (int i = 0; i < list->count; i++) {
        Implicant t = list->terms[i];
        strcat(buffer, "(");
        bool first_var = true;
        for (int bit = 0; bit < MAX_VARS; bit++) {
            if (!((t.mask >> bit) & 1)) {
                if (!first_var) strcat(buffer, " + ");
                char var = 'A' + bit;
                bool is_one = (t.value >> bit) & 1;
                char term[4];
                // Invert logic for POS
                if (is_one) sprintf(term, "%c'", var);
                else        sprintf(term, "%c", var);
                strcat(buffer, term);
                first_var = false;
            }
        }
        strcat(buffer, ")");
    }
}

/*
 * Function: Minimizer_GetMaxterms
 * -------------------------------
 * Inverts the truth table generation logic to find inputs that result in 0.
 */
TruthTable Minimizer_GetMaxterms(LogicNode* root) {
    TruthTable table;
    table.count = 0;
    if (!root) return table;
    for (int i = 0; i < 64; i++) {
        if (!AST_Evaluate(root, i)) { // Check for 0s
            table.minterms[table.count++] = i;
        }
    }
    return table;
}