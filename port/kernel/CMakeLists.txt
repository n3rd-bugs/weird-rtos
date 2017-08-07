# Add subdirectories.
if (${TGT_CPU} STREQUAL "atmega644")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/avr)
elseif (${TGT_CPU} STREQUAL "cortex-m4")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cortex-m4)
endif ()

# Add this directory to the include directory.
LIST(APPEND RTOS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})

# Update the project variables.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} CACHE INTERNAL "RTOS_INCLUDES" FORCE)
