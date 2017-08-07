# Make a list of all the files in this folder and append them to the ${RTOS_SOURCES}.
FILE(GLOB SOURCES ./*.c)
LIST(APPEND RTOS_SOURCES ${SOURCES})

# Add this directory to the include directory.
LIST(APPEND RTOS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})

# Update the project variables.
SET(RTOS_SOURCES ${RTOS_SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)
SET(RTOS_INCLUDES ${RTOS_INCLUDES} CACHE INTERNAL "RTOS_INCLUDES" FORCE)

# Add a configuration option for target tools.
setup_option_def(TOOL_TARGET "TOOL_AVR_GCC" MACRO "Target tools.")
setup_option_hide(TOOL_TARGET)