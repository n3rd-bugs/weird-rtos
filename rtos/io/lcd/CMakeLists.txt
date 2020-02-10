# LCD configurations.
setup_option_def(LCD_PCF8574 OFF DEFINE "Enable Alphanumeric LCD driver over PCF8574 GPIO controller.")

# Make a list of all the files in this folder and append them to the ${RTOS_SOURCES}.
FILE(GLOB SOURCES ./*.c)
set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)

# Add this directory to the include directory.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "RTOS_INCLUDES" FORCE)

# Inlcude configuration options.
include(${CMAKE_CURRENT_SOURCE_DIR}/lcd_an.cmake)