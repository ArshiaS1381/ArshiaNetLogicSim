/*
 * File: logic_parser.c
 * Version: 1.0.0
 * Description:
 * Implements a Shunting-Yard based parser for boolean expressions.
 * Converts infix mathematical notation (e.g., "A + B") into an 
 * Abstract Syntax Tree (AST).
 *
 * Features:
 * - Supports Implicit Multiplication: "AB" -> "A * B"
 * - Supports Postfix NOT: "A'" -> "NOT A"
 * - Handles operator precedence (NOT > AND > OR/XOR).
 */

#include "logic_parser.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_STACK 128

// Stack structure for building the AST nodes
typedef struct {
    LogicNode* nodes[MAX_STACK];
    int top;
} NodeStack;

// Stack structure for operators
typedef struct {
    char ops[MAX_STACK];
    int top;
} OpStack;

// --- Stack Helper Functions ---

void node_push(NodeStack* s, LogicNode* n) { if(s->top < MAX_STACK) s->nodes[s->top++] = n; }
LogicNode* node_pop(NodeStack* s) { return (s->top > 0) ? s->nodes[--s->top] : NULL; }

void op_push(OpStack* s, char c) { if(s->top < MAX_STACK) s->ops[s->top++] = c; }
char op_pop(OpStack* s) { return (s->top > 0) ? s->ops[--s->top] : 0; }
char op_peek(OpStack* s) { return (s->top > 0) ? s->ops[s->top - 1] : 0; }

/*
 * Function: get_precedence
 * ------------------------
 * Defines the order of operations for the parser.
 * Higher number = higher priority.
 */
int get_precedence(char op) {
    switch(op) {
        case '!': return 4; // NOT (Prefix)
        case '\'': return 4; // NOT (Postfix) - Highest priority
        case '*': return 3; // AND
        case '+': return 2; // OR
        case '^': return 2; // XOR
        case '(': return 0;
        default:  return 0;
    }
}

/*
 * Function: char_to_type
 * ----------------------
 * Maps character operators to AST node types.
 */
NodeType char_to_type(char op) {
    switch(op) {
        case '*': return NODE_AND;
        case '+': return NODE_OR;
        case '^': return NODE_XOR;
        case '!': return NODE_NOT;
        default:  return NODE_AND;
    }
}

/*
 * Function: build_subtree
 * -----------------------
 * Pops an operator and its operands, creates a new AST node,
 * and pushes it back onto the node stack.
 */
void build_subtree(NodeStack* nodes, OpStack* ops) {
    char op = op_pop(ops);
    LogicNode* node = AST_CreateNode(char_to_type(op));
    
    if (op == '!') {
        node->left = node_pop(nodes); // Unary Prefix
    } else {
        node->right = node_pop(nodes); // Binary (Right operand is on top)
        node->left = node_pop(nodes);
    }
    node_push(nodes, node);
}

/*
 * Function: Parser_ParseString
 * ----------------------------
 * The main parsing routine.
 * Iterates through the string token by token, managing the operator
 * and node stacks to build the tree.
 *
 * Returns NULL if the resulting tree is invalid or unbalanced.
 */
LogicNode* Parser_ParseString(const char* expression) {
    NodeStack nodes = { .top = 0 };
    OpStack ops = { .top = 0 };
    
    typedef enum { START, OP, VAR, OPEN_PAREN, CLOSE_PAREN } TokenType;
    TokenType last_token = START;

    const char* ptr = expression;
    while (*ptr) {
        char c = *ptr;
        
        if (isspace(c)) {
            ptr++;
            continue;
        }
        
        // Case 1: Variables (A-Z)
        if (isalnum(c)) {
            // Implicit Multiplication: insert '*' if preceding token was VAR or ')'
            if (last_token == VAR || last_token == CLOSE_PAREN) {
                while (ops.top > 0 && get_precedence(op_peek(&ops)) >= get_precedence('*')) {
                    build_subtree(&nodes, &ops);
                }
                op_push(&ops, '*');
            }
            node_push(&nodes, AST_CreateVar(toupper(c)));
            last_token = VAR;
        } 
        // Case 2: Postfix NOT (')
        else if (c == '\'') {
            // Immediately wrap the top node in a NOT node
            if (nodes.top > 0) {
                LogicNode* operand = node_pop(&nodes);
                LogicNode* notNode = AST_CreateNode(NODE_NOT);
                notNode->left = operand;
                node_push(&nodes, notNode);
                
                last_token = VAR; // Treat result as a variable for subsequent operations
            }
        }
        // Case 3: Open Parenthesis
        else if (c == '(') {
            if (last_token == VAR || last_token == CLOSE_PAREN) {
                while (ops.top > 0 && get_precedence(op_peek(&ops)) >= get_precedence('*')) {
                    build_subtree(&nodes, &ops);
                }
                op_push(&ops, '*');
            }
            op_push(&ops, c);
            last_token = OPEN_PAREN;
        } 
        // Case 4: Close Parenthesis
        else if (c == ')') {
            while (ops.top > 0 && op_peek(&ops) != '(') {
                build_subtree(&nodes, &ops);
            }
            op_pop(&ops); // Pop the '('
            last_token = CLOSE_PAREN;
        } 
        // Case 5: Binary Operators
        else {
            while (ops.top > 0 && get_precedence(op_peek(&ops)) >= get_precedence(c)) {
                // Right-associativity check for unary operators would go here
                if (op_peek(&ops) == '!' && c == '!') break; 
                build_subtree(&nodes, &ops);
            }
            op_push(&ops, c);
            last_token = OP;
        }
        ptr++;
    }
    
    // Clear remaining operators
    while (ops.top > 0) {
        build_subtree(&nodes, &ops);
    }
    
    // Final Validation: Stack should contain exactly one root node
    if (nodes.top != 1) {
        while (nodes.top > 0) AST_Free(node_pop(&nodes));
        return NULL; 
    }
    
    return nodes.nodes[0];
}