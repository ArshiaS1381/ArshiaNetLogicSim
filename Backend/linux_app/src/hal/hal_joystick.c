/*
 * File: hal_joystick.c
 * Version: 1.1.0
 * Description:
 * Hardware Abstraction Layer for the Analog Joystick via MCP3208 (SPI).
 * Reads ADC channels 0 (X) and 1 (Y) and resolves them into directional enums.
 */

#include "hal_joystick.h"
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// --- SPI Configuration ---
#define SPI_DEV        "/dev/spidev0.0"
#define SPI_SPEED      250000u
#define SPI_BITS       8u

// --- Calibration & Deadzone ---
#define DEADZONE       300
#define CENTER_VAL     2048

static int spi_fd = -1;

/*
 * Internal Helper: read_channel
 * -----------------------------
 * Sends the MCP3208 command byte to read a single-ended channel.
 */
static int read_channel(int ch) {
    if (spi_fd < 0) return -1;

    // MCP3208 Start Bit + SGL/DIFF + D2/D1/D0
    uint8_t tx[3] = { 
        (uint8_t)(0x06 | ((ch & 0x04) >> 2)),
        (uint8_t)((ch & 0x03) << 6), 
        0x00 
    };
    uint8_t rx[3] = {0};

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 3,
        .speed_hz = SPI_SPEED,
        .bits_per_word = SPI_BITS,
    };

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) return -1;

    // Result is in last 12 bits
    return ((rx[1] & 0x0F) << 8) | rx[2];
}

/*
 * Internal Helper: resolve_direction
 * ----------------------------------
 * Maps X/Y coordinates to a direction with deadzone hysteresis.
 */
static JoystickDir resolve_direction(int x, int y) {
    int dx = x - CENTER_VAL;
    int dy = y - CENTER_VAL; // Note: Y might need inversion depending on wiring

    // Check Deadzone
    if (abs(dx) < DEADZONE && abs(dy) < DEADZONE) return JOY_CENTER;

    // Determine dominant axis
    if (abs(dy) > abs(dx)) {
        return (dy > 0) ? JOY_DOWN : JOY_UP; // Assuming 0=Top, 4095=Bottom
    } else {
        return (dx > 0) ? JOY_RIGHT : JOY_LEFT;
    }
}

void HAL_Joystick_Init(void) {
    #ifdef BEAGLEY_BUILD
    spi_fd = open(SPI_DEV, O_RDWR);
    if (spi_fd < 0) {
        perror("[HAL Joystick] Failed to open SPI device");
        return;
    }
    printf("  [Joystick] Initialized (SPI)\n");
    #else
    printf("  [Joystick] Initialized (Sim)\n");
    #endif
}

JoystickDir HAL_Joystick_GetDir(void) {
    #ifdef BEAGLEY_BUILD
    if (spi_fd < 0) return JOY_CENTER;

    int x = read_channel(0);
    int y = read_channel(1);

    return resolve_direction(x, y);
    #else
    // Simulation Stub
    return JOY_CENTER;
    #endif
}

bool HAL_Joystick_IsPressed(void) {
    // Placeholder: If you wire the switch to a GPIO later, implement here.
    return false;
}