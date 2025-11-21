/**
 * ============================================================================
 * File: server.js
 * Version: 1.0.0
 * Description:
 * This is the main entry point for the Node.js backend. It acts as a "Bridge"
 * or middleware between the Web Frontend (Browser) and the C Logic Engine.
 * * Architecture:
 * 1. Web Server (Express): Serves the UI files (HTML/CSS/JS) on Port 8088.
 * 2. WebSocket Server (Socket.IO): Handles real-time, bi-directional communication
 * with the browser clients.
 * 3. UDP Socket (dgram): Handles low-latency communication with the C application.
 * * Data Flow:
 * Browser -> Socket.IO -> Node.js -> UDP -> C App (Logic Engine)
 * C App -> UDP -> Node.js -> Socket.IO -> Browser
 * ============================================================================
 */

const express = require('express');
const app = express();
const http = require('http');
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);
const dgram = require('dgram');

// --- CONFIGURATION CONSTANTS ---
const WEB_PORT = 8088;         // Port for the browser to access (http://localhost:8088)
const TARGET_IP = '127.0.0.1'; // Target IP of the C App. Use '127.0.0.1' for WSL/Linux, '192.168.7.2' for BeagleY-AI
const TARGET_PORT = 12345;     // UDP Port the C Application is listening on
const LISTEN_PORT = 12346;     // UDP Port Node.js listens on for responses from C

// --- UDP SOCKET SETUP (Backend-to-Backend Communication) ---
// We use UDP for its low overhead, matching the embedded nature of the C app.
const udpSocket = dgram.createSocket('udp4');

// Error handling for the UDP socket to prevent server crashes
udpSocket.on('error', (err) => {
    console.error(`UDP Error:\n${err.stack}`);
    udpSocket.close();
});

/**
 * UDP Message Handler
 * This function triggers whenever the C application sends a packet to Node.js.
 * It parses the JSON response and routes it to the correct WebSocket client.
 */
udpSocket.on('message', (msg, rinfo) => {
    const messageStr = msg.toString().trim();

    try {
        // Attempt to parse the incoming string as JSON
        const jsonData = JSON.parse(messageStr);

        // --- SESSION ROUTING LOGIC ---
        // The C app includes a 'uid' field if the response is meant for a specific user
        // (e.g., a private 'preview' command).
        if (jsonData.uid) {
            // Route exclusively to the specific socket ID (Browser Tab)
            io.to(jsonData.uid).emit('state_update', jsonData);
        } else {
            // If no 'uid' is present, this is a System Broadcast (e.g., Hardware State Changed).
            // Broadcast to ALL connected clients to keep them in sync.
            io.emit('state_update', jsonData);
        }
    } catch (e) {
        // Fallback: If the message isn't valid JSON, log it as a raw text message.
        // This is extremely useful for debugging C `printf` outputs remotely.
        io.emit('server_log', messageStr);
    }
});

// Bind the UDP socket to the listening port to start receiving data
udpSocket.bind(LISTEN_PORT, () => {
    console.log(`UDP Bridge Listening on port ${LISTEN_PORT}`);
});

/**
 * Helper: Send Command to C Backend
 * Formats the payload according to the protocol defined in the C application.
 * * Protocol Format: "SocketID|Command"
 * - SocketID: The unique ID of the browser user (for routing replies).
 * - Command: The text command (e.g., "program x A+B").
 */
function sendToCpp(socketId, command) {
    const payload = `${socketId}|${command}`;
    const message = Buffer.from(payload);

    // Send the packet to the C App (Localhost:12345)
    udpSocket.send(message, TARGET_PORT, TARGET_IP, (err) => {
        if (err) console.error('UDP Send Error:', err);
    });
}

// --- WEB SERVER SETUP ---
// Serve static assets (index.html, style.css, client-side JS) from the 'public' folder
app.use(express.static('public')); 

// --- SOCKET.IO SETUP (Frontend-to-Backend Communication) ---
io.on('connection', (socket) => {
    console.log(`User Connected: ${socket.id}`);

    // Event: 'command'
    // Triggered when the frontend calls socket.emit('command', ...)
    socket.on('command', (cmd) => {
        // Forward the command immediately to the C backend via UDP
        sendToCpp(socket.id, cmd);
    });

    socket.on('disconnect', () => {
        console.log(`User Disconnected: ${socket.id}`);
    });
});

// --- START SERVER ---
server.listen(WEB_PORT, () => {
    console.log(`------------------------------------------------`);
    console.log(`Web Interface running at: http://localhost:${WEB_PORT}`);
    console.log(`UDP Bridge Active: Node(:${LISTEN_PORT}) <-> C_App(:${TARGET_PORT})`);
    console.log(`------------------------------------------------`);
});