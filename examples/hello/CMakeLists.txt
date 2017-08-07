# Add minimum cmake requirement.
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Initialize example project.
project(hello VERSION "00.00.01" LANGUAGES C)

# Setup RTOS directory
set(RTOS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../ CACHE STRING "RTOS directory.")

# Add RTOS project.
add_subdirectory(${RTOS_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/rtos_build")

# Setup target.
set(PROJECT_SRCS "${CMAKE_SOURCE_DIR}/hello.c")
setup_target(${PROJECT_NAME} PROJECT_SRCS)
