# ==============================================================================
# File: aarch64-toolchain.cmake
# Description: Toolchain file for cross-compiling to BeagleY-AI (aarch64)
# ==============================================================================

# 1. Mandatory Toolchain Identification
# This file is for cross-compiling.
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR aarch64)

# 2. Specify the Compiler and Tools
# You MUST have the aarch64-linux-gnu toolchain installed on your host machine (WSL/Linux).
SET(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# 3. Define System Root (If using a sysroot for linked libraries)
# If your sysroot is installed at a specific location, uncomment the line below.
# SET(CMAKE_FIND_ROOT_PATH "/path/to/aarch64/sysroot") 

# Search for programs (like the compiler) in the host environment
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers only in the target environment (sysroot)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 4. Compiler Flags (Optional but Recommended for Cross-Compiling)
# Add any flags specific to the target hardware here (e.g., -mcpu=cortex-a72)
# SET(CMAKE_C_FLAGS_INIT "")