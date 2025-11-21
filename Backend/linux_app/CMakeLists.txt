# ==============================================================================
# File: CMakeLists.txt
# Version: 2.0.0
# Description:
#   Build configuration for the Digital Logic Simulation Engine.
#   Note: Simplified and refactored
# ==============================================================================

cmake_minimum_required(VERSION 3.10)

# 1. Environment Setup
project(LogicSim C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add compiler warnings for clean code
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
endif()

# Build option
option(BUILD_FOR_BEAGLEY "Enable BeagleY-AI specific hardware features" OFF)

# 2. Source Code Discovery
# ------------------------
# Finds ALL .c files in src/ and subdirectories (app, hal, logic, net, utils)
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(logic_sim ${SOURCES})

# 3. Header Include Paths
# -----------------------
# We must include all subdirectories so the compiler finds headers like "app_state.h"
target_include_directories(logic_sim PRIVATE 
    include
    include/hal
    src/app
    src/hal
    src/logic
    src/net
    src/utils
)

# 4. Conditional Linking
# ----------------------
if(BUILD_FOR_BEAGLEY)
    message(STATUS "Cross-Compiling for BeagleY-AI Hardware (ARM64)")
    
    target_compile_definitions(logic_sim PRIVATE BEAGLEY_BUILD)

    # --- LINKING METHOD: APT MULTIARCH ---
    # Looks for the libgpiod v2 you installed via 'apt install libgpiod-dev:arm64'
    find_library(GPIOD_LIB gpiod 
        PATHS /usr/lib/aarch64-linux-gnu
        NO_DEFAULT_PATH
    )

    if(NOT GPIOD_LIB)
        message(FATAL_ERROR "Could not find arm64 libgpiod! Did you run 'sudo apt install libgpiod-dev:arm64'?")
    endif()

    message(STATUS "Found ARM64 libgpiod: ${GPIOD_LIB}")

    # Link against the found library
    target_link_libraries(logic_sim PRIVATE ${GPIOD_LIB} pthread m)
else()
    message(STATUS "Building for Simulation (Stubs)")
    target_link_libraries(logic_sim PRIVATE pthread m)
endif()