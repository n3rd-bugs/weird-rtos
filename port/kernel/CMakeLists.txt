# Add subdirectories.
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/avr)

# Add this directory to the include directory.
LIST(APPEND RTOS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})

# Update the project variables.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} CACHE INTERNAL "RTOS_INCLUDES" FORCE)
