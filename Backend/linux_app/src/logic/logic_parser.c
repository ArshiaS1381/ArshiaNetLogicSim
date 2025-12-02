/*
 * File: logic_parser.c
 * Version: 1.1.0
 * Description:
 * Updated to parse '%' as NAND and '$' as NOR.
 */

#include "logic_parser.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_STACK 128

typedef struct {
    LogicNode* nodes[MAX_STACK];
    int top;
} NodeStack;

typedef struct {
    char ops[MAX_STACK];
    int top;
} OpStack;

void node_push(NodeStack* s, LogicNode* n) { if(s->top < MAX_STACK) s->nodes[s->top++] = n; }
LogicNode* node_pop(NodeStack* s) { return (s->top > 0) ? s->nodes[--s->top] : NULL; }

void op_push(OpStack* s, char c) { if(s->top < MAX_STACK) s->ops[s->top++] = c; }
char op_pop(OpStack* s) { return (s->top > 0) ? s->ops[--s->top] : 0; }
char op_peek(OpStack* s) { return (s->top > 0) ? s->ops[s->top - 1] : 0; }

int get_precedence(char op) {
    switch(op) {
        case '!': return 4; 
        case '\'': return 4; 
        case '*': return 3; // AND
        case '%': return 3; // NAND (Same as AND)
        case '+': return 2; // OR
        case '$': return 2; // NOR (Same as OR)
        case '^': return 2; 
        case '(': return 0;
        default:  return 0;
    }
}

NodeType char_to_type(char op) {
    switch(op) {
        case '*': return NODE_AND;
        case '+': return NODE_OR;
        case '^': return NODE_XOR;
        case '!': return NODE_NOT;
        case '%': return NODE_NAND; // New Token
        case '$': return NODE_NOR;  // New Token
        default:  return NODE_AND;
    }
}

void build_subtree(NodeStack* nodes, OpStack* ops) {
    char op = op_pop(ops);
    LogicNode* node = AST_CreateNode(char_to_type(op));
    
    if (op == '!') {
        node->left = node_pop(nodes);
    } else {
        node->right = node_pop(nodes);
        node->left = node_pop(nodes);
    }
    node_push(nodes, node);
}

LogicNode* Parser_ParseString(const char* expression) {
    NodeStack nodes = { .top = 0 };
    OpStack ops = { .top = 0 };
    typedef enum { START, OP, VAR, OPEN_PAREN, CLOSE_PAREN } TokenType;
    TokenType last_token = START;

    const char* ptr = expression;
    while (*ptr) {
        char c = *ptr;
        if (isspace(c)) { ptr++; continue; }
        
        if (isalnum(c)) {
            if (last_token == VAR || last_token == CLOSE_PAREN) {
                while (ops.top > 0 && get_precedence(op_peek(&ops)) >= get_precedence('*')) {
                    build_subtree(&nodes, &ops);
                }
                op_push(&ops, '*');
            }
            node_push(&nodes, AST_CreateVar(toupper(c)));
            last_token = VAR;
        } 
        else if (c == '\'') {
            if (nodes.top > 0) {
                LogicNode* operand = node_pop(&nodes);
                LogicNode* notNode = AST_CreateNode(NODE_NOT);
                notNode->left = operand;
                node_push(&nodes, notNode);
                last_token = VAR; 
            }
        }
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
        else if (c == ')') {
            while (ops.top > 0 && op_peek(&ops) != '(') {
                build_subtree(&nodes, &ops);
            }
            op_pop(&ops); 
            last_token = CLOSE_PAREN;
        } 
        else {
            // Handles *, +, ^, %, $
            while (ops.top > 0 && get_precedence(op_peek(&ops)) >= get_precedence(c)) {
                if (op_peek(&ops) == '!' && c == '!') break; 
                build_subtree(&nodes, &ops);
            }
            op_push(&ops, c);
            last_token = OP;
        }
        ptr++;
    }
    
    while (ops.top > 0) build_subtree(&nodes, &ops);
    
    if (nodes.top != 1) {
        while (nodes.top > 0) AST_Free(node_pop(&nodes));
        return NULL; 
    }
    return nodes.nodes[0];
}