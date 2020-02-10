# I2C based drivers.
setup_option_def(I2C_PCF8574 OFF DEFINE "Enable I2C based PCF8574 GPIO controller driver.")

# Make a list of all the files in this folder and append them to the ${RTOS_SOURCES}.
if (${IO_I2C})
    FILE(GLOB SOURCES ./i2c.c)
    set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)
endif()

if (${I2C_PCF8574})
    FILE(GLOB SOURCES ./pcf8574.c)
    set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)
endif()

# Add this directory to the include directory.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "RTOS_INCLUDES" FORCE)
