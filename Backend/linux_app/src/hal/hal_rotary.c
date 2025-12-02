/*
 * File: hal_rotary.c
 * Version: 2.4.0 (Libgpiod v2 + Detent Handling)
 * Description:
 * Handles the Rotary Encoder.
 * Fixes "Too Sensitive" issues by dividing quadrature steps.
 */

#include "hal_rotary.h"
#include <stdio.h>
#include <stdatomic.h>
#include <stdlib.h> // For abs()

#ifdef BEAGLEY_BUILD
    #include <pthread.h>
    #include <gpiod.h>
    #include <time.h>
    #include <unistd.h>

    // --- PIN CONFIGURATION ---
    #define ROT_CHIP "/dev/gpiochip2"
    #define PIN_DT   15 
    #define PIN_CLK  17 
    #define PIN_SW   18  

    #define CLICK_TIMEOUT_MS 300
    #define DEBOUNCE_MS      50
    
    // --- SENSITIVITY ADJUSTMENT ---
    // 4 = Standard (1 menu item per physical click)
    // 2 = Half-Step (Useful for some Alps encoders)
    // 1 = High Sensitivity (Raw)
    #define STEPS_PER_DETENT 4 

    static pthread_t rot_thread;
#endif

static atomic_int running = 0;
static atomic_int encoder_delta = 0;
static atomic_int btn_event = ROT_BTN_NONE;

#ifdef BEAGLEY_BUILD
static long long current_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

static void* rotary_loop(void* arg) {
    struct gpiod_chip* chip = gpiod_chip_open(ROT_CHIP);
    if (!chip) return NULL;

    struct gpiod_line_settings* settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_UP);
    gpiod_line_settings_set_edge_detection(settings, GPIOD_LINE_EDGE_BOTH);

    struct gpiod_line_config* line_cfg = gpiod_line_config_new();
    unsigned int offsets[] = {PIN_DT, PIN_CLK, PIN_SW};
    gpiod_line_config_add_line_settings(line_cfg, offsets, 3, settings);

    struct gpiod_request_config* req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "RotaryHAL");
    gpiod_request_config_set_event_buffer_size(req_cfg, 64);

    struct gpiod_line_request* req = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    gpiod_line_settings_free(settings);
    gpiod_line_config_free(line_cfg);
    gpiod_request_config_free(req_cfg);

    if (!req) {
        gpiod_chip_close(chip);
        return NULL;
    }

    // Logic State
    int last_encoded = 0;
    int last_sw = 1;
    long long last_press_time = 0;
    int click_count = 0;
    long long click_timer_start = 0;
    
    // Accumulator for sensitivity control
    int raw_accumulator = 0;

    struct gpiod_edge_event_buffer* ev_buf = gpiod_edge_event_buffer_new(64);

    while (atomic_load(&running)) {
        int ret = gpiod_line_request_wait_edge_events(req, 1000000000); 

        if (ret > 0) {
             gpiod_line_request_read_edge_events(req, ev_buf, 64);
        }

        int dt  = gpiod_line_request_get_value(req, PIN_DT);
        int clk = gpiod_line_request_get_value(req, PIN_CLK);
        int sw  = gpiod_line_request_get_value(req, PIN_SW);

        // --- Rotation Logic ---
        int encoded = (dt << 1) | clk;
        int sum = (last_encoded << 2) | encoded;

        // Valid CW transitions
        if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
            raw_accumulator++;
        }
        // Valid CCW transitions
        if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
            raw_accumulator--;
        }
        
        last_encoded = encoded;

        // --- Sensitivity Divider ---
        // Only update the public counter when we cross the threshold
        if (abs(raw_accumulator) >= STEPS_PER_DETENT) {
            if (raw_accumulator > 0) {
                atomic_fetch_add(&encoder_delta, 1);
                raw_accumulator -= STEPS_PER_DETENT;
            } else {
                atomic_fetch_sub(&encoder_delta, 1);
                raw_accumulator += STEPS_PER_DETENT;
            }
        }

        // --- Button Logic ---
        if (last_sw == 1 && sw == 0) { 
            long long now = current_ms();
            if (now - last_press_time > DEBOUNCE_MS) {
                click_count++;
                if (click_count == 1) click_timer_start = now;
                last_press_time = now;
            }
        }
        last_sw = sw;

        if (click_count > 0 && (current_ms() - click_timer_start > CLICK_TIMEOUT_MS)) {
            if (click_count >= 2) {
                atomic_store(&btn_event, ROT_BTN_DOUBLE_CLICK);
            } else {
                atomic_store(&btn_event, ROT_BTN_CLICK);
            }
            click_count = 0; 
        }
    }

    gpiod_edge_event_buffer_free(ev_buf);
    gpiod_line_request_release(req);
    gpiod_chip_close(chip);
    return NULL;
}
#endif

void HAL_Rotary_Init(void) {
    #ifdef BEAGLEY_BUILD
    atomic_store(&running, 1);
    if (pthread_create(&rot_thread, NULL, rotary_loop, NULL) != 0) {
        perror("[HAL Rotary] Failed to create thread");
    } else {
        printf("  [Rotary] Thread Started (Detent Mode: %d)\n", STEPS_PER_DETENT);
    }
    #else
    printf("  [Rotary] Sim Init\n");
    #endif
}

int HAL_Rotary_GetCount(void) {
    return atomic_exchange(&encoder_delta, 0);
}

RotaryButtonState HAL_Rotary_GetButtonEvent(void) {
    return atomic_exchange(&btn_event, ROT_BTN_NONE);
}

void HAL_Rotary_Cleanup(void) {
    #ifdef BEAGLEY_BUILD
    atomic_store(&running, 0);
    pthread_join(rot_thread, NULL);
    #endif
}