# Minimum CMake version
cmake_minimum_required(VERSION 3.26)

# Top-level project
project("Meltdown Engine")

# Library location for the application linking
set(meltdown_DIR "${CMAKE_CURRENT_BINARY_DIR}/Engine")

# Library and application directories
add_subdirectory("Engine")
add_subdirectory("Application")
