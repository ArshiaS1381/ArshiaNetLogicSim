/*
 * File: logic_ast.c
 * Version: 1.0.0
 * Description:
 * Memory management and core operations for the Abstract Syntax Tree.
 * Provides the fundamental building blocks for the logic engine, including
 * node allocation, recursive deallocation, debug printing, and evaluation.
 */

#include "logic_ast.h"
#include "utils_colors.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/*
 * Function: AST_CreateNode
 * ------------------------
 * Allocates a new operator node on the heap.
 * Returns NULL if allocation fails.
 */
LogicNode* AST_CreateNode(NodeType type) {
    LogicNode* node = (LogicNode*)malloc(sizeof(LogicNode));
    if (node) {
        node->type = type;
        node->var_name = 0;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

/*
 * Function: AST_CreateVar
 * -----------------------
 * Allocates a new variable node (leaf node).
 */
LogicNode* AST_CreateVar(char name) {
    LogicNode* node = AST_CreateNode(NODE_VAR);
    if (node) {
        node->var_name = name;
    }
    return node;
}

/*
 * Function: AST_Free
 * ------------------
 * Standard recursive freeing of the tree.
 * MUST be called on old trees before overwriting pointers to prevent leaks.
 */
void AST_Free(LogicNode* root) {
    if (!root) return;
    AST_Free(root->left);
    AST_Free(root->right);
    free(root);
}

// Helper for AST_Print
void print_indent(int level) {
    for (int i = 0; i < level; i++) printf("  ");
}

/*
 * Function: AST_Print
 * -------------------
 * Visual debug tool. Prints the tree structure to stdout using
 * indentation and colors to represent depth and node type.
 */
void AST_Print(LogicNode* root, int level) {
    if (!root) return;

    // Print pipes for tree hierarchy visualization
    for (int i = 0; i < level; i++) printf(C_CYAN "|   " C_RESET);
    if (level > 0) printf(C_CYAN "|-- " C_RESET);
    
    switch (root->type) {
        case NODE_VAR: 
            printf(C_B_GREEN "VAR(%c)" C_RESET "\n", root->var_name); 
            break;
        case NODE_AND: printf(C_B_BLUE "AND" C_RESET "\n"); break;
        case NODE_OR:  printf(C_B_MAGENTA "OR" C_RESET "\n"); break;
        case NODE_XOR: printf(C_B_YELLOW "XOR" C_RESET "\n"); break;
        case NODE_NOT: printf(C_B_RED "NOT" C_RESET "\n"); break;
        default:       printf("OP(%d)\n", root->type); break;
    }

    if (root->type == NODE_NOT) {
        AST_Print(root->left, level + 1); 
    } else if (root->type != NODE_VAR) {
        AST_Print(root->left, level + 1);
        AST_Print(root->right, level + 1);
    }
}

/*
 * Function: AST_Evaluate
 * ----------------------
 * Recursively computes the boolean value of the tree.
 *
 * root:       Current node.
 * input_mask: Bitmask where Bit 0 = 'A', Bit 1 = 'B', etc.
 */
bool AST_Evaluate(LogicNode* root, int input_mask) {
    if (!root) return false;

    // Base Case: Variable
    if (root->type == NODE_VAR) {
        int index = root->var_name - 'A'; 
        if (index < 0 || index > 31) return false;
        
        return (input_mask >> index) & 1;
    }

    // Recursive Step: Evaluate children
    bool left  = AST_Evaluate(root->left, input_mask);
    bool right = AST_Evaluate(root->right, input_mask);

    switch (root->type) {
        case NODE_AND: return left && right;
        case NODE_OR:  return left || right;
        case NODE_XOR: return left ^ right;
        case NODE_NOT: return !left; // Unary, ignores right child
        default: return false;
    }
}