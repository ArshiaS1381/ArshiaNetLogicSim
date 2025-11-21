/*
 * File: utils_timer.h
 * Version: 1.0.0
 * Description:
 * Provides high-resolution timing utilities for benchmarking and sequencing.
 * Essential for the verification suite where precise millisecond-level
 * delays and timestamping are required to validate logic propagation.
 */

#ifndef UTILS_TIMER_H
#define UTILS_TIMER_H

#include <stdbool.h>

/*
 * Function: Timer_Init
 * --------------------
 * Initializes the system clock or monotonic timer source.
 * This establishes the "zero" reference point for the application.
 */
void Timer_Init(void);

/*
 * Function: Timer_GetMillis
 * -------------------------
 * Retrieves the elapsed time since initialization (or system boot).
 *
 * Returns the number of milliseconds elapsed since Timer_Init().
 */
long long Timer_GetMillis(void);

/*
 * Function: Timer_SleepMs
 * -----------------------
 * Blocks execution of the calling thread for a specified duration.
 *
 * ms: The number of milliseconds to sleep.
 */
void Timer_SleepMs(int ms);

/*
 * Function: Timer_HasElapsed
 * --------------------------
 * Non-blocking helper to check if a time duration has passed.
 *
 * start_ts:    The timestamp (from Timer_GetMillis) when the event started.
 * duration_ms: The duration to wait for.
 * returns:     true if the duration has elapsed, false otherwise.
 */
bool Timer_HasElapsed(long long start_ts, int duration_ms);

#endif