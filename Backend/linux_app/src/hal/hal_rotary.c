/*
 * File: hal_rotary.c
 * Version: 2.3.0 (Libgpiod v2 + Threaded + Double Click)
 * Description:
 * Handles the Rotary Encoder using the modern libgpiod v2 API.
 * Monitors DT/CLK for rotation and SW for single/double clicks.
 */

#include "hal_rotary.h"
#include <stdio.h>
#include <stdatomic.h>

// If compiling for BeagleY, we use the real library.
// If compiling for Sim (WSL), we stub it out unless you explicitly link libgpiod.
#ifdef BEAGLEY_BUILD
    #include <pthread.h>
    #include <gpiod.h>
    #include <time.h>
    #include <unistd.h>

    // --- PIN CONFIGURATION ---
    // Verify these with 'gpioinfo' on your board!
    #define ROT_CHIP "/dev/gpiochip2"
    #define PIN_DT   5   // Header 29
    #define PIN_CLK  6   // Header 31
    #define PIN_SW   13  // Header 33

    #define CLICK_TIMEOUT_MS 300
    #define DEBOUNCE_MS      50

    static pthread_t rot_thread;
#endif

// Atomic state variables (Shared between thread and main loop)
static atomic_int running = 0;
static atomic_int encoder_delta = 0;
static atomic_int btn_event = ROT_BTN_NONE;

#ifdef BEAGLEY_BUILD
// Helper: Get current monotonic time in milliseconds
static long long current_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

// The Background Worker Thread
static void* rotary_loop(void* arg) {
    // 1. Open the GPIO Chip
    struct gpiod_chip* chip = gpiod_chip_open(ROT_CHIP);
    if (!chip) {
        perror("[HAL Rotary] Failed to open chip");
        return NULL;
    }

    // 2. Define Line Settings (Input, Pull-Up, Both Edges)
    struct gpiod_line_settings* settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_UP);
    gpiod_line_settings_set_edge_detection(settings, GPIOD_LINE_EDGE_BOTH);

    // 3. Add Pins to Configuration
    struct gpiod_line_config* line_cfg = gpiod_line_config_new();
    unsigned int offsets[] = {PIN_DT, PIN_CLK, PIN_SW};
    gpiod_line_config_add_line_settings(line_cfg, offsets, 3, settings);

    // 4. Configure the Request
    struct gpiod_request_config* req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "RotaryHAL");
    gpiod_request_config_set_event_buffer_size(req_cfg, 64);

    // 5. Request the Lines
    struct gpiod_line_request* req = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    // Cleanup config objects (request handles them now)
    gpiod_line_settings_free(settings);
    gpiod_line_config_free(line_cfg);
    gpiod_request_config_free(req_cfg);

    if (!req) {
        perror("[HAL Rotary] Failed to request lines");
        gpiod_chip_close(chip);
        return NULL;
    }

    // Logic State
    int last_encoded = 0;
    int last_sw = 1; // Pull-ups mean 1 is default (not pressed)
    long long last_press_time = 0;
    int click_count = 0;
    long long click_timer_start = 0;
    
    // Event Buffer
    struct gpiod_edge_event_buffer* ev_buf = gpiod_edge_event_buffer_new(64);

    while (atomic_load(&running)) {
        // Wait for events (1 sec timeout, just to let us check 'running')
        int ret = gpiod_line_request_wait_edge_events(req, 1000000000); 

        if (ret > 0) {
             // Read events to clear the kernel buffer
             gpiod_line_request_read_edge_events(req, ev_buf, 64);
        }

        // --- Read Live Values (v2 API) ---
        int dt  = gpiod_line_request_get_value(req, PIN_DT);
        int clk = gpiod_line_request_get_value(req, PIN_CLK);
        int sw  = gpiod_line_request_get_value(req, PIN_SW);

        // --- Rotation Logic (Quadrature) ---
        int encoded = (dt << 1) | clk;
        int sum = (last_encoded << 2) | encoded;

        // Check valid transitions
        if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) atomic_fetch_add(&encoder_delta, 1);
        if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) atomic_fetch_sub(&encoder_delta, 1);
        last_encoded = encoded;

        // --- Button Logic (Double Click) ---
        if (last_sw == 1 && sw == 0) { // Falling Edge (Press)
            long long now = current_ms();
            if (now - last_press_time > DEBOUNCE_MS) {
                click_count++;
                if (click_count == 1) click_timer_start = now;
                last_press_time = now;
            }
        }
        last_sw = sw;

        // Timer Check
        if (click_count > 0 && (current_ms() - click_timer_start > CLICK_TIMEOUT_MS)) {
            if (click_count >= 2) {
                atomic_store(&btn_event, ROT_BTN_DOUBLE_CLICK);
            } else {
                atomic_store(&btn_event, ROT_BTN_CLICK);
            }
            click_count = 0; // Reset
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
        printf("  [Rotary] Thread Started (Libgpiod v2)\n");
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