/*
 * File: hal_gpio.c
 * Version: 2.2.0 (Libgpiod v2 + Stubbed)
 * Description:
 * GPIO Driver using libgpiod v2 API.
 * Wraps all library calls in BEAGLEY_BUILD to allow simulation on WSL.
 */

#include "hal_gpio.h"
#include <stdio.h>

// Only include libgpiod headers if we are building for the real hardware
#ifdef BEAGLEY_BUILD
    #include <gpiod.h>
    #include <string.h>

    // --- CONFIGURATION ---
    // Default chip for the main header. 
    // Run 'gpioinfo' on your board (if available) or check schematics.
    // On BeagleY-AI, standard header pins are usually on gpiochip2.
    #define DEFAULT_CHIP "/dev/gpiochip2" 

    typedef struct {
        const char* chip_path;
        int line_offset;
        struct gpiod_line_request* req; // v2 Request Handle
        int direction;                  // 1 = Input, 2 = Output
    } GpioConfig;

    // The Master Lookup Table
    static GpioConfig pin_map[GPIO_COUNT] = {
        // Inputs (A-F) -> Header Pins 3, 5, 7, 8, 10, 11
        [GPIO_IN_A] = { DEFAULT_CHIP, 2,  NULL, GPIOD_LINE_DIRECTION_INPUT },
        [GPIO_IN_B] = { DEFAULT_CHIP, 3,  NULL, GPIOD_LINE_DIRECTION_INPUT },
        [GPIO_IN_C] = { DEFAULT_CHIP, 4,  NULL, GPIOD_LINE_DIRECTION_INPUT },
        [GPIO_IN_D] = { DEFAULT_CHIP, 14, NULL, GPIOD_LINE_DIRECTION_INPUT },
        [GPIO_IN_E] = { DEFAULT_CHIP, 15, NULL, GPIOD_LINE_DIRECTION_INPUT },
        [GPIO_IN_F] = { DEFAULT_CHIP, 17, NULL, GPIOD_LINE_DIRECTION_INPUT },

        // Outputs (X-W) -> Header Pins 36, 38, 40, 35
        [GPIO_OUT_X] = { DEFAULT_CHIP, 16, NULL, GPIOD_LINE_DIRECTION_OUTPUT },
        [GPIO_OUT_Y] = { DEFAULT_CHIP, 20, NULL, GPIOD_LINE_DIRECTION_OUTPUT },
        [GPIO_OUT_Z] = { DEFAULT_CHIP, 21, NULL, GPIOD_LINE_DIRECTION_OUTPUT },
        [GPIO_OUT_W] = { DEFAULT_CHIP, 19, NULL, GPIOD_LINE_DIRECTION_OUTPUT },
    };
#endif

// --- IMPLEMENTATION ---

void HAL_GPIO_Init(void) {
    #ifdef BEAGLEY_BUILD
    printf("  [GPIO] Init (libgpiod v2)\n");

    for (int i = 0; i < GPIO_COUNT; i++) {
        // 1. Open the specific chip for this pin
        struct gpiod_chip* chip = gpiod_chip_open(pin_map[i].chip_path);
        if (!chip) {
            // Silent error or print to stderr in real app
            continue;
        }

        // 2. Configure Line Settings
        struct gpiod_line_settings* settings = gpiod_line_settings_new();
        gpiod_line_settings_set_direction(settings, pin_map[i].direction);
        
        // Default Outputs to 0 (Low)
        if (pin_map[i].direction == GPIOD_LINE_DIRECTION_OUTPUT) {
            gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);
        }

        // 3. Configure Line Object
        struct gpiod_line_config* line_cfg = gpiod_line_config_new();
        unsigned int offsets[1] = { (unsigned int)pin_map[i].line_offset };
        gpiod_line_config_add_line_settings(line_cfg, offsets, 1, settings);

        // 4. Configure Request Object
        struct gpiod_request_config* req_cfg = gpiod_request_config_new();
        gpiod_request_config_set_consumer(req_cfg, "LogicSim_GPIO");

        // 5. Make the Request
        pin_map[i].req = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
        
        if (!pin_map[i].req) {
            fprintf(stderr, "[GPIO] Failed to request pin %d on chip %s\n", 
                    pin_map[i].line_offset, pin_map[i].chip_path);
        }

        // 6. Cleanup configuration objects
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(line_cfg);
        gpiod_request_config_free(req_cfg);
        
        // In v2, we can close the chip once the request is made
        gpiod_chip_close(chip); 
    }
    #else
    printf("  [GPIO] Init (Sim: 6 In, 4 Out)\n");
    #endif
}

bool HAL_GPIO_Read(GPIOPin pin) {
    if (pin >= GPIO_COUNT) return false;

    #ifdef BEAGLEY_BUILD
        if (pin_map[pin].req) {
            // In v2, get_value uses the offset relative to the request (which is 0 here since we requested 1 line)
            // But safer to use the absolute offset if the API supports it, or just request one by one.
            // Since we request 1 line per 'req', the offset inside the req is 0? 
            // Actually, gpiod_line_request_get_value takes the *offset within the request*.
            // Since we added exactly 1 line to this request, that line is at index 0 (not the hardware offset).
            // HOWEVER, libgpiod documentation implies you can pass the hardware offset if it was requested.
            // Let's look at the signature: int gpiod_line_request_get_value(struct gpiod_line_request *request, unsigned int offset);
            // It retrieves the value of the line with the given offset.
            
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