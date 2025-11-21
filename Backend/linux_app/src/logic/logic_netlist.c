/*
 * File: logic_netlist.c
 * Version: 1.0.0
 * Description:
 * Implements the Logic-to-Netlist conversion.
 * This module traverses the Abstract Syntax Tree (AST) and serializes it
 * into a JSON format compatible with graph visualization libraries (e.g., Cytoscape.js).
 *
 * It performs "flattening" of associative operators (A & B & C) to simplify
 * the visual representation.
 */

#include "logic_netlist.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
 * Function: append
 * ----------------
 * Helper to safely append a string to the main buffer.
 * Checks bounds to prevent buffer overflow.
 */
static void append(char* buffer, int* offset, int max_len, const char* str) {
    int len = strlen(str);
    if (*offset + len < max_len - 1) {
        strcpy(buffer + *offset, str);
        *offset += len;
    }
}

/*
 * Function: is_associative
 * ------------------------
 * Returns true if the operator supports flattening.
 * e.g., (A AND B) AND C  ==  A AND B AND C.
 */
static bool is_associative(NodeType type) {
    return (type == NODE_AND || type == NODE_OR || type == NODE_XOR);
}

// Forward declaration for recursive traversal
static int traverse(LogicNode* node, char* buffer, int* offset, int max_len, int* id_counter);

/*
 * Function: collect_inputs
 * ------------------------
 * Recursively gathers inputs for an associative operator.
 * If a child node has the same type as the parent (e.g., AND inside AND),
 * it merges the child's inputs directly into the parent, skipping the child node.
 */
static void collect_inputs(LogicNode* node, int parent_id, NodeType parent_type, 
                           char* buffer, int* offset, int max_len, int* id_counter) {
    if (!node) return;

    // Case 1: Flatten - Child is same operator as parent
    if (node->type == parent_type && is_associative(parent_type)) {
        collect_inputs(node->left, parent_id, parent_type, buffer, offset, max_len, id_counter);
        collect_inputs(node->right, parent_id, parent_type, buffer, offset, max_len, id_counter);
    } 
    // Case 2: Standard - Child is different, create a new edge
    else {
        int child_id = traverse(node, buffer, offset, max_len, id_counter);
        
        char edge_json[128];
        sprintf(edge_json, "{ \"data\": { \"source\": \"n%d\", \"target\": \"n%d\" } },", child_id, parent_id);
        append(buffer, offset, max_len, edge_json);
    }
}

/*
 * Function: traverse
 * ------------------
 * Recursive function to visit nodes and generate JSON elements.
 *
 * returns: The unique integer ID of the processed node.
 */
static int traverse(LogicNode* node, char* buffer, int* offset, int max_len, int* id_counter) {
    if (!node) return -1;

    int my_id = (*id_counter)++;
    char temp[256];

    // 1. Determine visual label
    char label[16];
    switch (node->type) {
        case NODE_VAR: sprintf(label, "%c", node->var_name); break;
        case NODE_AND: sprintf(label, "AND"); break;
        case NODE_OR:  sprintf(label, "OR"); break;
        case NODE_XOR: sprintf(label, "XOR"); break;
        case NODE_NOT: sprintf(label, "NOT"); break;
        default: sprintf(label, "?"); break;
    }

    // 2. Generate Node JSON
    sprintf(temp, "{ \"data\": { \"id\": \"n%d\", \"label\": \"%s\", \"type\": \"%s\" } },", 
            my_id, label, (node->type == NODE_VAR) ? "var" : "gate");
    append(buffer, offset, max_len, temp);

    // 3. Process Children (with flattening logic)
    if (node->type == NODE_NOT) {
        if (node->left) {
            int child_id = traverse(node->left, buffer, offset, max_len, id_counter);
            sprintf(temp, "{ \"data\": { \"source\": \"n%d\", \"target\": \"n%d\" } },", child_id, my_id);
            append(buffer, offset, max_len, temp);
        }
    } else if (node->type != NODE_VAR) {
        collect_inputs(node->left, my_id, node->type, buffer, offset, max_len, id_counter);
        collect_inputs(node->right, my_id, node->type, buffer, offset, max_len, id_counter);
    }

    return my_id;
}

/*
 * Function: Netlist_GenerateJSON
 * ------------------------------
 * Entry point for generating a netlist for a single output.
 */
void Netlist_GenerateJSON(const char* target_name, LogicNode* root, char* buffer, int max_len) {
    int offset = 0;
    int id_counter = 0;

    append(buffer, &offset, max_len, "[");

    if (root) {
        int root_id = traverse(root, buffer, &offset, max_len, &id_counter);

        // Create the final Output Node (e.g., "X")
        char temp[256];
        int out_id = id_counter++;
        sprintf(temp, "{ \"data\": { \"id\": \"n%d\", \"label\": \"%s\", \"type\": \"output\" } },", 
                out_id, target_name);
        append(buffer, &offset, max_len, temp);

        // Link logic root to output node
        sprintf(temp, "{ \"data\": { \"source\": \"n%d\", \"target\": \"n%d\" } },", root_id, out_id);
        append(buffer, &offset, max_len, temp);
    }

    // Handle trailing comma validity
    if (offset > 1 && buffer[offset-1] == ',') {
        offset--; 
    }

    append(buffer, &offset, max_len, "]");
    buffer[offset] = '\0'; 
}

/*
 * Function: Netlist_GenerateCombinedJSON
 * --------------------------------------
 * Entry point for generating the unified 4-channel netlist.
 * Iterates through all 4 roots and appends them to the same JSON array.
 */
void Netlist_GenerateCombinedJSON(
    const char* n1, LogicNode* r1,
    const char* n2, LogicNode* r2,
    const char* n3, LogicNode* r3,
    const char* n4, LogicNode* r4,
    char* buffer, int max_len) 
{
    int offset = 0;
    int id_counter = 0;
    char temp[256];

    append(buffer, &offset, max_len, "[");

    // Macro to process a single tree within the combined list
    #define ADD_TREE(name, root) \
        if (root) { \
            int root_id = traverse(root, buffer, &offset, max_len, &id_counter); \
            int out_id = id_counter++; \
            sprintf(temp, "{ \"data\": { \"id\": \"n%d\", \"label\": \"%s\", \"type\": \"output\" } },", out_id, name); \
            append(buffer, &offset, max_len, temp); \
            sprintf(temp, "{ \"data\": { \"source\": \"n%d\", \"target\": \"n%d\" } },", root_id, out_id); \
            append(buffer, &offset, max_len, temp); \
        }

    ADD_TREE(n1, r1);
    ADD_TREE(n2, r2);
    ADD_TREE(n3, r3);
    ADD_TREE(n4, r4);

    if (offset > 1 && buffer[offset-1] == ',') offset--; 
    append(buffer, &offset, max_len, "]");
    buffer[offset] = '\0'; 
}