# GPIO based drivers.
setup_option_def(CONFIG_DHT OFF DEFINE "Enable DHT temperature sensor driver.")

# Make a list of all the required files in this folder and append them to the ${RTOS_SOURCES}.
if (${CONFIG_DHT})
    FILE(GLOB SOURCES ./dhtxx.c)
    set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)
endif()

# Add this directory to the include directory.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "RTOS_INCLUDES" FORCE)