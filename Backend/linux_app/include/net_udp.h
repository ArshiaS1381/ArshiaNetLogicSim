/*
 * File: net_udp.h
 * Version: 1.0.0
 * Description:
 * Manages the UDP networking interface for the Logic Server.
 * Handles broadcasting state updates to listeners and sending
 * specific data payloads like netlists or analysis results.
 *
 * Note: Version 1.1.0 will expand this to include hardware-in-the-loop
 * communication protocols.
 */

#ifndef NET_UDP_H
#define NET_UDP_H

#include "logic_ast.h"

/*
 * Function: NetUDP_Init
 * ---------------------
 * Initializes the UDP socket, sets up broadcast permissions, and
 * binds to the configured port. Must be called at startup.
 */
void NetUDP_Init(void);

/*
 * Function: NetUDP_Cleanup
 * ------------------------
 * Closes the socket and releases networking resources.
 * Should be called during the application shutdown sequence.
 */
void NetUDP_Cleanup(void);

/*
 * Function: NetUDP_BroadcastState
 * -------------------------------
 * Sends the current application state (Inputs/Outputs) to all
 * devices on the local subnet listening on the target port.
 */
void NetUDP_BroadcastState(void);

/*
 * Function: NetUDP_SendLogicResult
 * --------------------------------
 * Transmits the results of the logic minimization process to a specific target.
 *
 * target:   IP address or hostname of the recipient.
 * sop:      The Sum-of-Products equation string.
 * pos:      The Product-of-Sums equation string.
 * minterms: Array of minterm integers.
 * count:    Number of items in the minterms array.
 * mode:     The current operational mode identifier.
 */
void NetUDP_SendLogicResult(const char* target, const char* sop, const char* pos, const int* minterms, int count, const char* mode);

/*
 * Function: NetUDP_SendNetlist
 * ----------------------------
 * Transmits the circuit topology (JSON Netlist) to a specific target.
 *
 * target:    IP address or hostname of the recipient.
 * json_data: The complete JSON string describing the circuit.
 */
void NetUDP_SendNetlist(const char* target, const char* json_data);

/*
 * Function: NetUDP_SendRaw
 * ------------------------
 * Sends a raw data string over UDP. Useful for debugging or custom protocols.
 *
 * json_data: The raw string payload to transmit.
 */
void NetUDP_SendRaw(const char* json_data);

#endif