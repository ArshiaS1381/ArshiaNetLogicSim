/*
 * File: utils_timer.c
 * Version: 1.0.0
 * Description:
 * Implements high-resolution timekeeping functions for Linux.
 * Uses the POSIX `CLOCK_REALTIME` to provide millisecond-precision
 * timestamps and sleep functionality.
 */

#include "utils_timer.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// Base timestamp for relative time calculations
static long long start_time_ms = 0;

/*
 * Function: get_time_raw
 * ----------------------
 * Internal helper to get current system time in milliseconds.
 */
static long long get_time_raw(void) {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
}

/*
 * Function: Timer_Init
 * --------------------
 * Establishes the "zero" time point (epoch) for the application.
 */
void Timer_Init(void) {
    start_time_ms = get_time_raw();
}

/*
 * Function: Timer_GetMillis
 * -------------------------
 * Returns the number of milliseconds elapsed since Timer_Init() was called.
 */
long long Timer_GetMillis(void) {
    return get_time_raw() - start_time_ms;
}

/*
 * Function: Timer_SleepMs
 * -----------------------
 * Wrapper for nanosleep to pause execution for a specific duration.
 */
void Timer_SleepMs(int ms) {
    struct timespec req;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&req, NULL);
}