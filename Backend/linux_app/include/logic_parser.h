/*
 * File: logic_parser.h
 * Version: 1.0.0
 * Description:
 * Provides the recursive descent parser for boolean expressions.
 * Converts user-friendly strings (e.g., "A * (B + C')") into
 * pointer-based Abstract Syntax Trees (AST).
 *
 * Supported syntax includes:
 * - Variables: A-F
 * - Operators: * (AND), + (OR), ^ (XOR), ! or ' (NOT)
 * - Parentheses for grouping.
 */

#ifndef LOGIC_PARSER_H
#define LOGIC_PARSER_H

#include "logic_ast.h"

/*
 * Function: Parser_ParseString
 * ----------------------------
 * The main entry point for the parsing engine.
 *
 * expression: The null-terminated string containing the boolean equation.
 *
 * returns:    A pointer to the root LogicNode of the generated tree.
 * Returns NULL if a syntax error is encountered (e.g., unbalanced parens).
 */
LogicNode* Parser_ParseString(const char* expression);

#endif