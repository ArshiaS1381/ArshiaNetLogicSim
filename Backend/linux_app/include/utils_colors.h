/*
 * File: utils_colors.h
 * Version: 1.0.0
 * Description:
 * Definitions for ANSI escape codes to enable colored text in the terminal.
 * Used primarily for debugging and log highlighting (e.g., Red for errors,
 * Green for success).
 */

#ifndef UTILS_COLORS_H
#define UTILS_COLORS_H

// Reset to default terminal color
#define C_RESET   "\033[0m"

// Standard Colors
#define C_RED     "\033[0;31m"
#define C_GREEN   "\033[0;32m"
#define C_YELLOW  "\033[0;33m"
#define C_BLUE    "\033[0;34m"
#define C_MAGENTA "\033[0;35m"
#define C_CYAN    "\033[0;36m"
#define C_WHITE   "\033[0;37m"

// Bold / Bright Colors
#define C_B_RED     "\033[1;31m"
#define C_B_GREEN   "\033[1;32m"
#define C_B_YELLOW  "\033[1;33m"
#define C_B_BLUE    "\033[1;34m"
#define C_B_MAGENTA "\033[1;35m"
#define C_B_CYAN    "\033[1;36m"
#define C_B_WHITE   "\033[1;37m"

#endif