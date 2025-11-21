# ==============================================================================
# File: CMakeLists.txt
# Version: 1.1.0
# Description:
#   Build configuration for the Digital Logic Simulation Engine.
# ==============================================================================

cmake_minimum_required(VERSION 3.10)

# 1. Environment Setup
# --------------------
# Define the project name and language (C).
project(LogicSim C)

# Enforce the C99 standard.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add compiler warnings.
# if(CMAKE_COMPILER_IS_GNUCC)
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
# endif()

# Add a build option for the BeagleY-AI target.
option(BUILD_FOR_BEAGLEY "Enable BeagleY-AI specific hardware features" OFF)

# 2. Source Code Discovery
# ------------------------
# Auto-discover all general source files.
file(GLOB_RECURSE SOURCES "src/*.c")

# Create the executable target.
add_executable(logic_sim "")

# 3. Header Include Paths
# -----------------------
# Use target_include_directories for better encapsulation.
target_include_directories(logic_sim PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal"
)

# 4. Conditional Compilation and Linking
# -----------------------------------------

# Always discover and add HAL sources (needed for linking the stub functions).
file(GLOB_RECURSE HAL_SOURCES "src/hal/*.c")
target_sources(logic_sim PRIVATE ${HAL_SOURCES})

if(BUILD_FOR_BEAGLEY)
    message(STATUS "BeagleY-AI build enabled. Enabling BEAGLEY_BUILD macro.")
    # Add a preprocessor definition for conditional compilation in C code.
    target_compile_definitions(logic_sim PRIVATE BEAGLEY_BUILD)
else()
    message(STATUS "BeagleY-AI build disabled. Using HAL simulation stubs.")
endif()

# Add all other sources. (This SOURCES list no longer needs to be filtered)
target_sources(logic_sim PRIVATE ${SOURCES})

# 5. Library Linking (unchanged)
# ------------------
# Link against pthread.
target_link_libraries(logic_sim pthread)

# Optional: Print status message
# message(STATUS "Configured LogicSim build with sources: ${SOURCES}")
