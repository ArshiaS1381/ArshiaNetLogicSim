/*
 * File: hal_general.h
 * Version: 1.0.0
 * Description:
 * General system hardware initialization and management.
 * Acts as the master entry point for bringing up all hardware subsystems
 * in the correct order.
 */

#ifndef HAL_GENERAL_H
#define HAL_GENERAL_H

/*
 * Function: HAL_General_Init
 * --------------------------
 * Master initialization routine.
 * Calls the Init functions for GPIO, I2C, SPI, and IPC subsystems.
 * Returns only when hardware is ready for operation.
 */
void HAL_General_Init(void);

/*
 * Function: HAL_General_Cleanup
 * -----------------------------
 * Master shutdown routine.
 * Ensures all hardware peripherals are put into a safe state before
 * the application exits.
 */
void HAL_General_Cleanup(void);

/*
 * Function: HAL_General_GetBoardTemp
 * ----------------------------------
 * Reads the on-board temperature sensor.
 * Useful for system health monitoring.
 *
 * returns: Temperature in degrees Celsius.
 */
float HAL_General_GetBoardTemp(void);

#endif