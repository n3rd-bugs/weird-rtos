# Make a list of all the files in this folder and append them to the ${RTOS_SOURCES}.
FILE(GLOB SOURCES ./*.c)
set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)

# Add this directory to the include directory.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "RTOS_INCLUDES" FORCE)

# Inlcude configuration options.
include(${CMAKE_CURRENT_SOURCE_DIR}/assert.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/global.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/idle.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/kernel.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/scheduler.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/tasks.cmake)