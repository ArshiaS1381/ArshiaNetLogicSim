/*
 * File: logic_ast.c
 * Version: 1.1.0
 * Description:
 * Updated evaluation logic for NAND/NOR.
 */

#include "logic_ast.h"
#include "utils_colors.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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

LogicNode* AST_CreateVar(char name) {
    LogicNode* node = AST_CreateNode(NODE_VAR);
    if (node) {
        node->var_name = name;
    }
    return node;
}

void AST_Free(LogicNode* root) {
    if (!root) return;
    AST_Free(root->left);
    AST_Free(root->right);
    free(root);
}

void AST_Print(LogicNode* root, int level) {
    if (!root) return;

    for (int i = 0; i < level; i++) printf(C_CYAN "|   " C_RESET);
    if (level > 0) printf(C_CYAN "|-- " C_RESET);
    
    switch (root->type) {
        case NODE_VAR:  printf(C_B_GREEN "VAR(%c)" C_RESET "\n", root->var_name); break;
        case NODE_AND:  printf(C_B_BLUE "AND" C_RESET "\n"); break;
        case NODE_OR:   printf(C_B_MAGENTA "OR" C_RESET "\n"); break;
        case NODE_XOR:  printf(C_B_YELLOW "XOR" C_RESET "\n"); break;
        case NODE_NOT:  printf(C_B_RED "NOT" C_RESET "\n"); break;
        case NODE_NAND: printf(C_B_RED "NAND" C_RESET "\n"); break; // New
        case NODE_NOR:  printf(C_B_MAGENTA "NOR" C_RESET "\n"); break; // New
        default:        printf("OP(%d)\n", root->type); break;
    }

    if (root->type == NODE_NOT) {
        AST_Print(root->left, level + 1); 
    } else if (root->type != NODE_VAR) {
        AST_Print(root->left, level + 1);
        AST_Print(root->right, level + 1);
    }
}

bool AST_Evaluate(LogicNode* root, int input_mask) {
    if (!root) return false;

    if (root->type == NODE_VAR) {
        int index = root->var_name - 'A'; 
        if (index < 0 || index > 31) return false;
        return (input_mask >> index) & 1;
    }

    bool left  = AST_Evaluate(root->left, input_mask);
    bool right = AST_Evaluate(root->right, input_mask);

    switch (root->type) {
        case NODE_AND: return left && right;
        case NODE_OR:  return left || right;
        case NODE_XOR: return left ^ right;
        case NODE_NOT: return !left;
        case NODE_NAND: return !(left && right); // New
        case NODE_NOR:  return !(left || right); // New
        default: return false;
    }
}