/*
 * File: logic_ast.h
 * Version: 1.1.0
 * Description:
 * Updated to support NAND and NOR nodes.
 */

#ifndef LOGIC_AST_H
#define LOGIC_AST_H

#include <stdbool.h>

/*
 * Enum: NodeType
 * --------------
 * Enumerates the supported logic gate types and atomic elements.
 *
 * NODE_VAR:  Represents a raw input variable (A, B, C, D, E, F).
 * NODE_AND:  Represents a logical AND gate (*).
 * NODE_OR:   Represents a logical OR gate (+).
 * NODE_XOR:  Represents a logical XOR gate (^).
 * NODE_NOT:  Represents a logical NOT/Inverter (! or ').
 * NODE_NAND: Represents a NAND gate.
 * NODE_NOR:  Represents a NOR gate.
 * NODE_XNOR: Represents an XNOR gate.
 */
typedef enum {
    NODE_VAR,
    NODE_AND,
    NODE_OR,
    NODE_XOR,
    NODE_NOT,
    NODE_NAND, // New
    NODE_NOR   // New
} NodeType;

/*
 * Struct: LogicNode
 * -----------------
 * A single node in the Abstract Syntax Tree.
 *
 * type:     The specific operation or variable this node represents.
 * var_name: If type is NODE_VAR, this holds the character identifier (e.g., 'A').
 * Ignored for operator nodes.
 * left:     Pointer to the left child node (Operand 1).
 * right:    Pointer to the right child node (Operand 2).
 * (Note: NODE_NOT uses only the left child; NODE_VAR uses neither).
 */
typedef struct LogicNode {
    NodeType type;
    char var_name; // Only for NODE_VAR
    struct LogicNode* left;
    struct LogicNode* right;
} LogicNode;

/*
 * Function: AST_CreateNode
 * ------------------------
 * Allocates heap memory for a new operator node.
 * Initial children pointers are set to NULL.
 *
 * type:    The type of logic gate to create.
 *
 * returns: Pointer to the new LogicNode, or NULL if allocation fails.
 */
LogicNode* AST_CreateNode(NodeType type);

/*
 * Function: AST_CreateVar
 * -----------------------
 * Allocates heap memory for a new variable node.
 *
 * name:    The character identifier for the variable (e.g., 'A', 'B').
 *
 * returns: Pointer to the new LogicNode configured as NODE_VAR.
 */
LogicNode* AST_CreateVar(char name);

/*
 * Function: AST_Free
 * ------------------
 * Recursively traverses the tree and frees all allocated memory.
 * This is crucial to prevent memory leaks when equations are updated.
 *
 * root: Pointer to the root of the tree to delete.
 */
void AST_Free(LogicNode* root);

/*
 * Function: AST_Print
 * -------------------
 * Debug helper that prints the tree structure to stdout visually.
 * Useful for verifying that the parser built the tree correctly.
 *
 * root:  Pointer to the tree to print.
 * level: The current indentation depth (start with 0).
 */
void AST_Print(LogicNode* root, int level);

/*
 * Function: AST_Evaluate
 * ----------------------
 * Computes the boolean result of the logic tree for a specific input state.
 *
 * root:       Pointer to the logic tree to evaluate.
 * input_mask: An integer bitmask representing the state of all inputs.
 * Bit 0 = Input A, Bit 1 = Input B, etc.
 *
 * returns:    true (1) if the logic evaluates to High, false (0) for Low.
 */
bool AST_Evaluate(LogicNode* root, int input_mask);

#endif