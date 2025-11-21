/*
 * File: app_utils.c
 * Version: 1.0.0
 * Description:
 * High-level utility logic for the application.
 * This module orchestrates the data flow between the Parser, Minimizer,
 * and Network modules. It abstracts the complex sequence of compiling
 * a logic string into a broadcastable result.
 */

#include "app_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_state.h"
#include "net_udp.h"
#include "logic_ast.h"
#include "logic_parser.h"
#include "logic_minimizer.h"
#include "logic_netlist.h"
#include "utils_colors.h" 

/*
 * Function: Process_Equation
 * --------------------------
 * The core compilation pipeline:
 * 1. Parse string -> AST.
 * 2. AST -> Truth Table.
 * 3. Truth Table -> Minimized SOP (Quine-McCluskey).
 * 4. AST -> Maxterms -> Minimized POS.
 * 5. AST -> JSON Netlist.
 * 6. Broadcast results via UDP.
 *
 * Returns true if the entire pipeline succeeded.
 */
bool Process_Equation(const char* label, const char* expression, const char* mode) {
    // Empty expression is technically valid (Logic 0) but we skip processing
    if (strlen(expression) == 0) return true;

    // Step 1: Parse
    LogicNode* root = Parser_ParseString(expression);
    if (root) {
        // Step 2: SOP Minimization
        TruthTable tt = Minimizer_GenerateTruthTable(root);
        ImplicantList primes = Minimizer_FindPrimeImplicants(tt);
        char sop_buffer[512];
        Minimizer_PrintSOP(&primes, sop_buffer);
        
        // Step 3: POS Minimization (via Maxterms)
        TruthTable maxterms = Minimizer_GetMaxterms(root);
        ImplicantList zero_primes = Minimizer_FindPrimeImplicants(maxterms);
        char pos_buffer[512];
        Minimizer_PrintPOS(&zero_primes, pos_buffer);

        // Step 4: Send Analysis Data
        NetUDP_SendLogicResult(label, sop_buffer, pos_buffer, tt.minterms, tt.count, mode);

        // Step 5: Generate and Send Visualization Data
        char netlist_json[8192]; 
        Netlist_GenerateJSON(label, root, netlist_json, sizeof(netlist_json));
        NetUDP_SendNetlist(label, netlist_json);

        // Cleanup
        AST_Free(root);
        return true;
    }
    return false;
}

/*
 * Function: Send_Combined_Update
 * ------------------------------
 * Generates a unified view of the system for the "Combined" UI tab.
 * It parses all four channels, generates a composite JSON netlist,
 * and aggregates the truth tables into a single packet.
 *
 * Note: This function allocates large buffers on the static segment to
 * avoid stack overflow.
 */
void Send_Combined_Update(const char* in_x, const char* in_y, const char* in_z, const char* in_w) {
    // Parse all inputs temporarily
    LogicNode* rX = Parser_ParseString(in_x);
    LogicNode* rY = Parser_ParseString(in_y);
    LogicNode* rZ = Parser_ParseString(in_z);
    LogicNode* rW = Parser_ParseString(in_w);
    
    static char combined_netlist[65536]; 
    Netlist_GenerateCombinedJSON("X", rX, "Y", rY, "Z", rZ, "W", rW, combined_netlist, sizeof(combined_netlist));

    TruthTable tX = Minimizer_GenerateTruthTable(rX);
    TruthTable tY = Minimizer_GenerateTruthTable(rY);
    TruthTable tZ = Minimizer_GenerateTruthTable(rZ);
    TruthTable tW = Minimizer_GenerateTruthTable(rW);
    
    static char packet[131072]; 
    int offset = 0;

    // Start JSON object
    offset += sprintf(packet + offset, "{ \"type\": \"combined\", ");
    
    // Helper macro to format JSON arrays for minterms
    #define APPEND_ARR(key, tt) \
        offset += sprintf(packet+offset, "\"%s\": [", key); \
        for(int i=0; i<tt.count; i++) { \
            offset += sprintf(packet+offset, "%d", tt.minterms[i]); \
            if(i < tt.count-1) offset += sprintf(packet+offset, ","); \
        } \
        offset += sprintf(packet+offset, "], ");

    APPEND_ARR("mintermsX", tX);
    APPEND_ARR("mintermsY", tY);
    APPEND_ARR("mintermsZ", tZ);
    APPEND_ARR("mintermsW", tW);

    // Append the netlist graph
    offset += sprintf(packet+offset, "\"elements\": %s }", combined_netlist);

    NetUDP_SendRaw(packet);

    // Clean up temporary trees
    if(rX) AST_Free(rX); if(rY) AST_Free(rY);
    if(rZ) AST_Free(rZ); if(rW) AST_Free(rW);
}

/*
 * Function: Process_Stateless
 * ---------------------------
 * "Preview" Mode.
 * Processes an equation for display without saving it to the persistent
 * app state. This allows users to type and see real-time updates
 * without overwriting their saved configuration.
 */
void Process_Stateless(const char* label, const char* expression) {
    printf(C_B_CYAN "  [Stateless] Previewing %s: \"%s\"...\n" C_RESET, label, expression);
    
    // Run standard processing with "preview" mode tag
    Process_Equation(label, expression, "preview");

    // Create a temporary view of the world where 'label' is replaced by 'expression'
    // but other channels remain as they are in the state.
    SharedState st = AppState_GetSnapshot();
    const char *x = st.input_x, *y = st.input_y, *z = st.input_z, *w = st.input_w;
    
    if (strcmp(label, "x") == 0) x = expression;
    else if (strcmp(label, "y") == 0) y = expression;
    else if (strcmp(label, "z") == 0) z = expression;
    else if (strcmp(label, "w") == 0) w = expression;

    Send_Combined_Update(x, y, z, w);
}