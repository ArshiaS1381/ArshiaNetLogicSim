/*
 * File: hal_gpio.c
 * Version: 2.7.1
 * Description:
 * GPIO Driver with startup config printing.
 * FIXED: Unterminated #ifdef error and updated pins for BeagleY-AI.
 */

#include "hal_gpio.h"
#include <stdio.h>

#ifdef BEAGLEY_BUILD
    #include <gpiod.h>
    #include <string.h>

    // --- CONFIGURATION ---
    #define DEFAULT_CHIP "/dev/gpiochip2" 

    typedef struct {
        const char* chip_path;
        int line_offset;
        struct gpiod_line_request* req; 
        int direction;                  
    } GpioConfig;

    // Mapping Table
    static GpioConfig pin_map[GPIO_COUNT] = {
        // Inputs (A-D) -> Header Pins 3, 5, 7, 8 (GPIO 2, 3, 4, 14)
        [GPIO_IN_A] = { DEFAULT_CHIP, 2,  NULL, GPIOD_LINE_DIRECTION_INPUT },
        [GPIO_IN_B] = { DEFAULT_CHIP, 3,  NULL, GPIOD_LINE_DIRECTION_INPUT },
        [GPIO_IN_C] = { DEFAULT_CHIP, 4,  NULL, GPIOD_LINE_DIRECTION_INPUT },
        [GPIO_IN_D] = { DEFAULT_CHIP, 14, NULL, GPIOD_LINE_DIRECTION_INPUT },

        // --- CONFLICT RESOLUTION ---
        // Moved from 15/17 (Rotary) to 22/23 (Free) based on your gpioinfo
        [GPIO_IN_E] = { DEFAULT_CHIP, 22, NULL, GPIOD_LINE_DIRECTION_INPUT }, 
        [GPIO_IN_F] = { DEFAULT_CHIP, 23, NULL, GPIOD_LINE_DIRECTION_INPUT },

        // Outputs (X-W) -> Header Pins 36, 38, 40, 35 (GPIO 16, 20, 21, 19)
        [GPIO_OUT_X] = { DEFAULT_CHIP, 16, NULL, GPIOD_LINE_DIRECTION_OUTPUT },
        [GPIO_OUT_Y] = { DEFAULT_CHIP, 20, NULL, GPIOD_LINE_DIRECTION_OUTPUT },
        [GPIO_OUT_Z] = { DEFAULT_CHIP, 21, NULL, GPIOD_LINE_DIRECTION_OUTPUT },
        [GPIO_OUT_W] = { DEFAULT_CHIP, 19, NULL, GPIOD_LINE_DIRECTION_OUTPUT },
    };

    // Helper names for printing
    static const char* PIN_NAMES[GPIO_COUNT] = {
        "IN A", "IN B", "IN C", "IN D", "IN E", "IN F",
        "OUT X", "OUT Y", "OUT Z", "OUT W"
    };

#endif // Ends the Config Section

// --- IMPLEMENTATION ---

void HAL_GPIO_Init(void) {
    #ifdef BEAGLEY_BUILD
    printf("  [GPIO] Init (libgpiod v2)\n");
    printf("  -------------------------------------------------\n");
    printf("  | %-6s | %-14s | %-4s | %-6s |\n", "Label", "Chip", "Line", "Status");
    printf("  -------------------------------------------------\n");

    for (int i = 0; i < GPIO_COUNT; i++) {
        // Skip disabled pins
        if (pin_map[i].line_offset < 0) {
            printf("  | %-6s | %-14s | %-4s | %-6s |\n", 
                PIN_NAMES[i], "---", "---", "DISABLED");
            continue;
        }

        // 1. Open Chip
        struct gpiod_chip* chip = gpiod_chip_open(pin_map[i].chip_path);
        if (!chip) {
            printf("  | %-6s | %-14s | %-4d | %-6s |\n", 
                PIN_NAMES[i], pin_map[i].chip_path, pin_map[i].line_offset, "ERR:CHIP");
            continue;
        }

        // 2. Configure Settings
        struct gpiod_line_settings* settings = gpiod_line_settings_new();
        gpiod_line_settings_set_direction(settings, pin_map[i].direction);
        
        if (pin_map[i].direction == GPIOD_LINE_DIRECTION_OUTPUT) {
            gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);
        }

        // 3. Configure Line
        struct gpiod_line_config* line_cfg = gpiod_line_config_new();
        unsigned int offsets[1] = { (unsigned int)pin_map[i].line_offset };
        gpiod_line_config_add_line_settings(line_cfg, offsets, 1, settings);

        // 4. Request
        struct gpiod_request_config* req_cfg = gpiod_request_config_new();
        gpiod_request_config_set_consumer(req_cfg, "LogicSim_GPIO");

        pin_map[i].req = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
        
        // Print Status Row
        if (pin_map[i].req) {
             printf("  | %-6s | %-14s | %-4d | %-6s |\n", 
                PIN_NAMES[i], pin_map[i].chip_path, pin_map[i].line_offset, "OK");
        } else {
             printf("  | %-6s | %-14s | %-4d | %-6s |\n", 
                PIN_NAMES[i], pin_map[i].chip_path, pin_map[i].line_offset, "BUSY/ERR");
        }

        // Cleanup
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(line_cfg);
        gpiod_request_config_free(req_cfg);
        gpiod_chip_close(chip); 
    }
    printf("  -------------------------------------------------\n");
    #else
    printf("  [GPIO] Init (Sim: 6 In, 4 Out)\n");
    #endif // Ends the Init Function block (This #endif was missing!)
}

bool HAL_GPIO_Read(GPIOPin pin) {
    if (pin >= GPIO_COUNT) return false;

    #ifdef BEAGLEY_BUILD
        if (pin_map[pin].req) {
            return gpiod_line_request_get_value(pin_map[pin].req, pin_map[pin].line_offset) == GPIOD_LINE_VALUE_ACTIVE;
        }
    #endif
    return false;
}

void HAL_GPIO_Write(GPIOPin pin, bool value) {
    if (pin >= GPIO_COUNT) return;

    #ifdef BEAGLEY_BUILD
        if (pin_map[pin].req) {
            gpiod_line_request_set_value(pin_map[pin].req, pin_map[pin].line_offset, 
                value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
        }
    #endif
}

void HAL_GPIO_Cleanup(void) {
    #ifdef BEAGLEY_BUILD
    for (int i = 0; i < GPIO_COUNT; i++) {
        if (pin_map[i].req) {
            gpiod_line_request_release(pin_map[i].req);
            pin_map[i].req = NULL;
        }
    }
    #endif
}