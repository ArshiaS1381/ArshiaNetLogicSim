/*
 * File: hal_led.c
 * Version: 1.1.0
 * Description:
 * Hardware Abstraction Layer for system LEDs.
 * Controls the On-Board BeagleY-AI LEDs (ACT/PWR) via sysfs.
 */

#include "hal_led.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define PATH_GREEN "/sys/class/leds/ACT/brightness"
#define PATH_RED   "/sys/class/leds/PWR/brightness"

// Keep file handles open for performance
static FILE* f_green = NULL;
static FILE* f_red = NULL;

static void write_led(FILE* f, bool on) {
    if (!f) return;
    rewind(f);
    fprintf(f, "%d", on ? 255 : 0);
    fflush(f);
}

void HAL_LED_Init(void) {
    #ifdef BEAGLEY_BUILD
    f_green = fopen(PATH_GREEN, "w");
    f_red   = fopen(PATH_RED,   "w");

    if (!f_green || !f_red) {
        perror("[HAL LED] Failed to open LED sysfs paths");
    } else {
        // Turn off triggers so we can control manually
        FILE* f_trig = fopen("/sys/class/leds/ACT/trigger", "w");
        if (f_trig) { fprintf(f_trig, "none"); fclose(f_trig); }
        
        f_trig = fopen("/sys/class/leds/PWR/trigger", "w");
        if (f_trig) { fprintf(f_trig, "none"); fclose(f_trig); }

        printf("  [LED] Initialized (Sysfs)\n");
    }
    #else
    printf("  [LED] Initialized (Sim)\n");
    #endif
}

void HAL_LED_SetRGB(uint8_t r, uint8_t g, uint8_t b) {
    #ifdef BEAGLEY_BUILD
        // Map Red -> PWR LED, Green -> ACT LED
        write_led(f_red,   r > 127);
        write_led(f_green, g > 127);
    #else
        // Sim: Do nothing to avoid console spam
    #endif
}

void HAL_LED_SetChannel(int channel, bool state) {
    // Placeholder: Could be mapped to external LEDs if added later
}