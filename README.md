# Final Project: Digital Logic Simulation Engine

This project is a digital logic simulation engine with a web-based interface. It allows users to define and test digital logic equations, visualize the results, and interact with the simulation through a web browser or a UDP-based command interface. The project is designed to be cross-platform, with support for both x86-64 (Linux/WSL) and ARM64 (BeagleY-AI) architectures. When compiled for the BeagleY-AI, the project can interface with hardware components such as a joystick, LEDs, a rotary encoder, and GPIO pins.

## Table of Contents

- [Features](#features)
- [System Architecture](#system-architecture)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Build Arguments](#build-arguments)
  - [Toolchain](#toolchain)
  - [Build Instructions](#build-instructions)
  - [Running the Application](#running-the-application)
- [UDP Commands](#udp-commands)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Logic Simulation:** Define and simulate digital logic equations for up to four outputs (W, X, Y, Z).
- **Web Interface:** A user-friendly web interface for interacting with the simulation, visualizing logic circuits, and viewing results in real-time.
- **UDP Interface:** A simple UDP-based command interface for programmatic control of the simulation.
- **Cross-Compilation:** Support for both x86-64 and ARM64 architectures, with conditional compilation for hardware-specific features.
- **Hardware Integration (BeagleY-AI only):**
  - **Inputs:**
    - Joystick (SPI)
    - Rotary Encoder (GPIO)
    - 6 GPIO input pins
  - **Outputs:**
    - Onboard LEDs
    - 4 GPIO output pins
- **Multiple Modes of Operation:** Switch between different input and output sources (Web/UDP, Physical Controls, GPIO).

## System Architecture

The project consists of three main components:

1.  **C Logic Engine:** A lightweight, high-performance simulation engine written in C. It is responsible for parsing and evaluating logic equations, managing the simulation state, and interfacing with hardware components on the BeagleY-AI.
2.  **Node.js Bridge:** A Node.js server that acts as a bridge between the C Logic Engine and the Web Frontend. It communicates with the C engine via UDP and with the web frontend via WebSockets.
3.  **Web Frontend:** A web-based user interface built with HTML, CSS, and JavaScript. It allows users to interact with the simulation, view results, and visualize logic circuits in real-time.

### Data Flow

```
Browser <--> WebSockets <--> Node.js Bridge <--> UDP <--> C Logic Engine
```

## Getting Started

### Prerequisites

- **CMake:** Version 3.10 or higher.
  - To install on Debian/Ubuntu: `sudo apt-get install cmake`
- **GCC:** A C compiler (e.g., `gcc`).
  - To install on Debian/Ubuntu: `sudo apt-get install build-essential`
- **Node.js and npm:** For running the web server.
  - To install on Debian/Ubuntu: `sudo apt-get install nodejs npm`
- **concurrently:** A utility to run multiple commands concurrently.
  - To install: `npm install -g concurrently`
- **aarch64-linux-gnu-gcc (for cross-compilation only):** The toolchain for compiling for the BeagleY-AI.
    - To install on Debian/Ubuntu: `sudo apt-get install gcc-aarch64-linux-gnu`

### Build Arguments

The C Logic Engine build is controlled by the following CMake build argument:

-   `-DBUILD_FOR_BEAGLEY=ON|OFF`: This option controls the compilation target.
    -   `OFF` (default): Compiles the project for the host machine (x86-64). In this mode, all hardware-specific code is replaced with simulated stubs.
    -   `ON`: Compiles the project for the BeagleY-AI (aarch64). This enables the hardware abstraction layer and allows the application to interface with GPIO, SPI, and other hardware peripherals.

### Toolchain

The `aarch64-toolchain.cmake` file is used to configure the cross-compilation environment for the BeagleY-AI. It specifies the `aarch64-linux-gnu-gcc` compiler and sets the system name and processor type. When you run the `npm run build:beagley` command, this toolchain file is automatically used to build the C Logic Engine.

### Build Instructions

#### For x86-64 (Linux/WSL)

1.  **Create a build directory:**
    ```bash
    mkdir -p Backend/build
    ```
2.  **Configure the project with CMake:**
    ```bash
    cd Backend/build
    cmake ../linux_app
    ```
3.  **Compile the project:**
    ```bash
    make
    ```

#### For BeagleY-AI (ARM64)

1.  **Create a build directory:**
    ```bash
    mkdir -p Backend/build
    ```
2.  **Configure the project with CMake, enabling the `BUILD_FOR_BEAGLEY` option:**
    ```bash
    cd Backend/build
    cmake ../linux_app -DBUILD_FOR_BEAGLEY=ON -DCMAKE_TOOLCHAIN_FILE=../../linux_app/aarch64-toolchain.cmake
    ```
3.  **Compile the project:**
    ```bash
    make
    ```

### Running the Application

1.  **Start the Node.js web server and the C application:**
    ```bash
    cd Frontend/node_server
    npm install
    npm start
    ```
2.  **Open your web browser and navigate to `http://localhost:8088`**

## UDP Commands

The UDP server listens on port `12345`. Commands can be sent as plain text strings.

- `login <pass>`: Authenticate for admin access.
- `program <target> <eq>`: Set a persistent logic equation for a target (w, x, y, z).
- `preview <target> <eq>`: Test an equation without saving.
- `kmap <target> <csv>`: Program a target using a comma-separated list of minterms.
- `print <target>`: Print the current equation for a target.
- `clear`: Clear all programmed equations.
- `refresh`: Force a broadcast of the current state.
- `help`: Display a list of available commands.

## Project Structure

```
Final/
├── Backend/
│   ├── CMakeLists.txt
│   ├── admin/
│   │   ├── admin.secret
│   │   └── set_password.py
│   ├── build/
│   └── linux_app/
│       ├── aarch64-toolchain.cmake
│       ├── CMakeLists.txt
│       ├── include/
│       └── src/
└── Frontend/
    └── node_server/
        ├── package.json
        ├── server.js
        └── public/
```

- **`Backend/`**: The C-based logic simulation engine.
  - **`CMakeLists.txt`**: The root CMake file for the backend.
  - **`admin/`**: Scripts for managing the UDP interface password.
  - **`build/`**: The build directory for the C application.
  - **`linux_app/`**: The source code for the C application.
    - **`aarch64-toolchain.cmake`**: The toolchain file for cross-compiling to the BeagleY-AI.
    - **`CMakeLists.txt`**: The CMake file for the C application.
    - **`include/`**: Header files for the C application.
    - **`src/`**: Source code for the C application.
- **`Frontend/`**: The Node.js-based web server and front-end.
  - **`node_server/`**: The Node.js application.
    - **`package.json`**: Node.js project file.
    - **`server.js`**: The main server file.
    - **`public/`**: The web front-end (HTML, CSS, JavaScript).

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue.

## License

This project is licensed under the ISC License. See the `LICENSE` file for details.
