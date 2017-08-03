# Add minimum cmake requirement.
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Initialize RTOS project.
project(w3rd-rtos VERSION "00.00.01" LANGUAGES C)

# Setup verbose option.
set(BUILD_VERBOSE OFF CACHE BOOL "Verbose build mode.")

# Setup output directory option.
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}${CMAKE_BUILD_TYPE} CACHE STRING "Output directory.")

# Update CMAKE configuration.
set(CMAKE_VERBOSE_MAKEFILE ${BUILD_VERBOSE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# Update CMAKE configuration for the parent if any.
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if (HAS_PARENT)
    set(CMAKE_VERBOSE_MAKEFILE ${BUILD_VERBOSE} PARENT_SCOPE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR} PARENT_SCOPE)
endif ()

# Include helpers.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/helper.cmake)

# Initialize global variables.
set(RTOS_SOURCES "" CACHE INTERNAL "RTOS_SOURCES" FORCE)
set(RTOS_LINK_SOURCES "" CACHE INTERNAL "RTOS_LINK_SOURCES" FORCE)
set(RTOS_INCLUDES "" CACHE INTERNAL "RTOS_INCLUDES" FORCE)
set(RTOS_DEFS -DCMAKE_BUILD CACHE INTERNAL "RTOS_DEFS" FORCE)
set(RTOS_LIB ${PROJECT_NAME} CACHE INTERNAL "RTOS_LIB" FORCE)

# Add RTOS folders.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/api)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/port)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rtos)

# Create an RTOS library from the collected sources.
add_definitions(${RTOS_DEFS})
add_library(${RTOS_LIB} STATIC ${RTOS_SOURCES})
target_include_directories(${RTOS_LIB} PUBLIC ${RTOS_INCLUDES})
