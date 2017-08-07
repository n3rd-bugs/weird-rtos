# Make a list of all the files in this folder and append them to the ${RTOS_SOURCES}.
FILE(GLOB SOURCES ./*.c)
set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)

# Add this directory to the include directory.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "RTOS_INCLUDES" FORCE)

# Inlcude configuration options.
include(${CMAKE_CURRENT_SOURCE_DIR}/assert_options.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/global_options.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/idle_options.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/kernel_options.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/scheduler_options.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/tasks_options.cmake)