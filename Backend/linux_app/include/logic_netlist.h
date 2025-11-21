/*
 * File: logic_netlist.h
 * Version: 1.0.0
 * Description:
 * Handles the generation of JSON-formatted netlists.
 * A "Netlist" in this context is a serialized representation of the
 * logic tree structure, intended for the front-end UI to render
 * circuit diagrams visually.
 */

#ifndef LOGIC_NETLIST_H
#define LOGIC_NETLIST_H

#include "logic_ast.h"

/*
 * Function: Netlist_GenerateJSON
 * ------------------------------
 * Serializes a single logic tree into a JSON object.
 *
 * target_name: The name of the output (e.g., "Output X").
 * root:        Pointer to the AST root for this output.
 * buffer:      Buffer to store the resulting JSON string.
 * max_len:     Maximum capacity of the buffer.
 */
void Netlist_GenerateJSON(const char* target_name, LogicNode* root, char* buffer, int max_len);

/*
 * Function: Netlist_GenerateCombinedJSON
 * --------------------------------------
 * Aggregates the logic trees for all four primary outputs (X, Y, Z, W)
 * into a single unified JSON structure. This allows the client to
 * visualize the entire system state in one pass.
 *
 * n1, r1: Name and Root Node for the first circuit (X).
 * n2, r2: Name and Root Node for the second circuit (Y).
 * n3, r3: Name and Root Node for the third circuit (Z).
 * n4, r4: Name and Root Node for the fourth circuit (W).
 * buffer: Output buffer for the JSON string.
 * max_len: Buffer size limit.
 */
void Netlist_GenerateCombinedJSON(
    const char* n1, LogicNode* r1,
    const char* n2, LogicNode* r2,
    const char* n3, LogicNode* r3,
    const char* n4, LogicNode* r4,
    char* buffer, int max_len
);

#endif