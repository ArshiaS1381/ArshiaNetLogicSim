/*
 * File: net_udp.c
 * Version: 1.1.0
 * Description:
 * Implements the UDP server thread for external communication.
 * Listens on Port 12345 for ASCII commands and handles the specific
 * logic for "Input Protection" when in GPIO mode.
 */

#include "net_udp.h"
#include "app_state.h"
#include "net_json.h"
#include "logic_minimizer.h" 
#include "logic_program.h"   
#include "app_utils.h"       
#include "utils_colors.h"    
#include "app_verification.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// --- Configuration Constants ---
#define PORT_LISTEN  12345
#define PORT_NODEJS  12346
#define IP_NODEJS    "127.0.0.1" 

// --- Global Variables (Module Level) ---
static pthread_t udp_thread;
static int sockfd;
static struct sockaddr_in node_addr;
static volatile int running = 1;
static volatile int exit_requested = 0;
static char current_uid[64] = ""; 

// --- Security Globals ---
static unsigned long ADMIN_HASH = 0; 
static const char* SECRET_FILE = "admin/admin.secret";

/*
 * Function: hash_string
 * ---------------------
 * DJB2 Hash Algorithm. Used for password verification.
 * Must match the implementation used in the generation script.
 */
static unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    return hash & 0xFFFFFFFF; 
}

/*
 * Function: load_admin_secret
 * ---------------------------
 * Loads the admin password hash from a local file.
 * Falls back to a default hash if the file is missing.
 */
static void load_admin_secret() {
    FILE* f = fopen(SECRET_FILE, "r");
    if (f) {
        char buf[32];
        if (fgets(buf, sizeof(buf), f)) {
            ADMIN_HASH = strtoul(buf, NULL, 10);
            printf(C_B_GREEN "[Security] Admin Hash Loaded: %lu" C_RESET "\n", ADMIN_HASH);
        }
        fclose(f);
    } else {
        ADMIN_HASH = 2088290703; // Default "1234"
        printf(C_B_RED "[Security] Warning: 'admin.secret' not found. Defaulting." C_RESET "\n");
    }
}

/*
 * Function: send_packet
 * ---------------------
 * Transmits a JSON string to the configured Node.js backend.
 * If a User ID (UID) is active, it wraps the JSON to target that specific user session.
 */
static void send_packet(const char* json_body) {
    char buffer[65536];
    
    // Wrapper logic for multi-user support
    if (strlen(current_uid) > 0) {
        sprintf(buffer, "{ \"uid\": \"%s\", %s", current_uid, json_body + 1);
    } else {
        strcpy(buffer, json_body);
    }

    sendto(sockfd, buffer, strlen(buffer), 0, 
           (const struct sockaddr *)&node_addr, sizeof(node_addr));
}

/*
 * Function: process_command
 * -------------------------
 * Parses raw ASCII commands received over UDP.
 * Format: "UID|Command Arguments"
 *
 * Supported Commands:
 * - login <pass>: Authenticate admin access.
 * - program <target> <eq>: Set persistent equation.
 * - preview <target> <eq>: Test equation without saving.
 * - kmap <target> <csv>: Program via minterms.
 * - print/clear/refresh: Utility commands.
 */
static void process_command(char* raw_msg) {
    // Split UID and Command
    char* pipe_ptr = strchr(raw_msg, '|');
    char* cmd;
    
    if (pipe_ptr) {
        *pipe_ptr = '\0'; 
        strncpy(current_uid, raw_msg, 63);
        cmd = pipe_ptr + 1;
    } else {
        current_uid[0] = '\0';
        cmd = raw_msg;
    }

    // Strip newline characters
    cmd[strcspn(cmd, "\r\n")] = 0;
    printf(C_B_MAGENTA "[UDP]" C_RESET " User " C_CYAN "[%s]" C_RESET " sent: " C_YELLOW "'%s'" C_RESET "\n", 
           current_uid, cmd);

    // --- Authentication ---
    if (strncmp(cmd, "login ", 6) == 0) {
        char* attempt = cmd + 6;
        unsigned long attempt_hash = hash_string(attempt);
        
        if (attempt_hash == ADMIN_HASH) {
            send_packet("{ \"type\": \"auth\", \"status\": \"success\" }");
            printf("      " C_B_GREEN "✔ AUTH SUCCESS" C_RESET "\n");
        } else {
            send_packet("{ \"type\": \"auth\", \"status\": \"fail\" }");
            printf("      " C_B_RED "✘ AUTH FAILED" C_RESET "\n");
        }
        return;
    }

    // --- INPUT CONTROL (New for Requirement #10) ---
    else if (strncmp(cmd, "set_input ", 10) == 0) {
        SharedState st = AppState_GetSnapshot();
        
        // PROTECTION: If in GPIO Mode, hardware pins rule. Web cannot override.
        if (st.mode == MODE_GPIO_EXEC) {
             send_packet("{ \"log\": \"Error: System is in GPIO Mode. Inputs are locked to hardware pins.\" }");
             printf("      " C_B_RED "✘ DENIED:" C_RESET " GPIO Mode Active\n");
        } else {
             int mask = atoi(cmd + 10);
             AppState_SetInputMask((uint8_t)mask);
             send_packet("{ \"status\": \"Inputs Updated\" }");
        }
    }

    // --- Stateless Preview ---
    else if (strncmp(cmd, "preview ", 8) == 0) {
        char* ptr = cmd + 8;
        char target[2] = { ptr[0], '\0' }; 
        Process_Stateless(target, ptr + 2);
    }
    // --- K-Map Preview ---
    else if (strncmp(cmd, "preview_kmap ", 13) == 0) {
        char* ptr = cmd + 13;
        char target[2] = { ptr[0], '\0' };
        
        if (ptr[1] == ' ') {
            char* csv = ptr + 2;
            TruthTable tt; tt.count = 0;
            char* token = strtok(csv, ",");
            while (token != NULL && tt.count < 64) {
                tt.minterms[tt.count++] = atoi(token);
                token = strtok(NULL, ",");
            }
            
            ImplicantList primes = Minimizer_FindPrimeImplicants(tt);
            char sop_buffer[512];
            Minimizer_PrintSOP(&primes, sop_buffer);

            printf("      " C_BLUE "↳ K-Map Reversal:" C_RESET " %s\n", sop_buffer);
            Process_Stateless(target, sop_buffer);
        }
    }
    // --- Persistent Programming ---
    else if (strncmp(cmd, "program x ", 10) == 0) { AppState_SetInputX(cmd + 10); send_packet("{ \"status\": \"Updated X\" }"); }
    else if (strncmp(cmd, "program y ", 10) == 0) { AppState_SetInputY(cmd + 10); send_packet("{ \"status\": \"Updated Y\" }"); }
    else if (strncmp(cmd, "program z ", 10) == 0) { AppState_SetInputZ(cmd + 10); send_packet("{ \"status\": \"Updated Z\" }"); }
    else if (strncmp(cmd, "program w ", 10) == 0) { AppState_SetInputW(cmd + 10); send_packet("{ \"status\": \"Updated W\" }"); }
    
    else if (strncmp(cmd, "kmap ", 5) == 0) {
        char* ptr = cmd + 5;
        char target[2] = { ptr[0], '\0' };
        Program_From_Minterms(target, ptr + 2);
        send_packet("{ \"status\": \"Processing K-Map Input\" }");
    }
    // --- Utilities ---
    else if (strcmp(cmd, "print x") == 0) {
        SharedState st = AppState_GetSnapshot();
        char buf[256]; snprintf(buf, sizeof(buf), "{ \"log\": \"X = %s\" }", st.input_x); send_packet(buf);
    }
    else if (strcmp(cmd, "clear") == 0) {
        AppState_SetInputX(""); AppState_SetInputY(""); 
        AppState_SetInputZ(""); AppState_SetInputW("");
        send_packet("{ \"status\": \"Cleared All\" }");
    }
    else if (strcmp(cmd, "refresh") == 0) {
        AppState_Touch();
        printf("[UDP] Force Refresh Requested\n");
    }
    else if (strcmp(cmd, "exit") == 0) {
        exit_requested = 1; 
        running = 0;        
        AppState_Touch(); 
        send_packet("{ \"status\": \"Shutting down.\" }");
    }

    // --- Help Command ---
    else if (strcmp(cmd, "help") == 0) {
        const char* help_json = 
            "{ \"type\": \"help\", \"commands\": ["
            "\"set_input <mask> - Set inputs A-F (0-63). Locked in GPIO Mode.\","
            "\"program <target> <eq> - Set equation for x/y/z/w.\","
            "\"preview <target> <eq> - Test equation.\","
            "\"kmap <target> <csv> - Program via minterms.\""
            "] }";
        send_packet(help_json);
    }
    else {
        printf("      " C_B_RED "✘ ERROR:" C_RESET " Unknown command\n");
        char err_buf[256];
        snprintf(err_buf, sizeof(err_buf), "{ \"log\": \"Error: Unknown command '%s'\" }", cmd);
        send_packet(err_buf);
    }
}

/*
 * Function: udp_loop
 * ------------------
 * The main execution loop for the UDP thread.
 * Blocks on recvfrom() until data arrives.
 */
static void* udp_loop(void* arg) {
    char buffer[1024];
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    while (running) {
        int n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&cliaddr, &len);
        if (n > 0) {
            buffer[n] = '\0';
            process_command(buffer);
        }
    }
    return NULL;
}

// --- Public API Implementation ---

void NetUDP_Init(void) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { perror("Socket failed"); exit(EXIT_FAILURE); }
    
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT_LISTEN);

    // Allow port reuse for faster restarts
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { perror("Bind failed"); exit(EXIT_FAILURE); }

    memset(&node_addr, 0, sizeof(node_addr));
    node_addr.sin_family = AF_INET;
    node_addr.sin_port = htons(PORT_NODEJS);
    inet_pton(AF_INET, IP_NODEJS, &node_addr.sin_addr);

    load_admin_secret();
    
    pthread_create(&udp_thread, NULL, udp_loop, NULL);
    printf("[UDP] Server listening on port %d\n", PORT_LISTEN);
}

void NetUDP_Cleanup(void) {
    running = 0;
    pthread_cancel(udp_thread);
    pthread_join(udp_thread, NULL);
    close(sockfd);
}

void NetUDP_BroadcastState(void) {
    char json_buf[1024];
    SharedState st = AppState_GetSnapshot();
    JSON_SerializeState(&st, json_buf, sizeof(json_buf));
    current_uid[0] = '\0'; 
    send_packet(json_buf);
}

void NetUDP_SendLogicResult(const char* target, const char* sop, const char* pos, const int* minterms, int count, const char* mode) {
    char buffer[4096]; 
    int offset = 0;
    
    offset += sprintf(buffer + offset, 
        "{ \"type\": \"result\", \"mode\": \"%s\", \"target\": \"%s\", \"sop\": \"%s\", \"pos\": \"%s\", \"minterms\": [", 
        mode, target, sop, pos);

    for (int i = 0; i < count; i++) {
        offset += sprintf(buffer + offset, "%d", minterms[i]);
        if (i < count - 1) offset += sprintf(buffer + offset, ", ");
    }
    offset += sprintf(buffer + offset, "] }");
    
    send_packet(buffer);
}

void NetUDP_SendNetlist(const char* target, const char* json_data) {
    static char buffer[8192]; 
    snprintf(buffer, sizeof(buffer), "{ \"type\": \"netlist\", \"target\": \"%s\", \"elements\": %s }", target, json_data);
    send_packet(buffer);
}

void NetUDP_SendRaw(const char* json_data) {
    send_packet(json_data);
}

int NetUDP_ExitRequested(void) {
    return exit_requested;
}