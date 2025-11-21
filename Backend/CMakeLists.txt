# ==============================================================================
# File: CMakeLists.txt
# Version: 1.1.0
# Description:
#   Root build configuration for the Final project.
#   This file sets up the overall project environment and includes the
#   Linux application subdirectory.
# ==============================================================================

# 1. Project Setup
# ----------------
# Require CMake 3.10 or newer.
cmake_minimum_required(VERSION 3.10)

# Define the project name.
project(FinalProject)

# 2. Add Subdirectories
# ---------------------
# Include the Linux application. CMake will process the CMakeLists.txt
# in that directory.
add_subdirectory(linux_app)
