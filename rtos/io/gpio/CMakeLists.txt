# GPIO based drivers.
setup_option_def(GPIO_1WIRE OFF DEFINE "Enable 1Wire protocol driver.")
setup_option_def(GPIO_DHT OFF DEFINE "Enable DHT temperature sensor driver.")
setup_option_def(GPIO_DS182X OFF DEFINE "Enable DS182X temperature sensor driver.")
setup_option_def(GPIO_MAX7219 OFF DEFINE "Enable MAX7219 interface driver.")

# Make a list of all the required files in this folder and append them to the ${RTOS_SOURCES}.
if (${GPIO_1WIRE})
    FILE(GLOB SOURCES ./1wire.c)
    set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)
endif()

if (${GPIO_DHT})
    FILE(GLOB SOURCES ./dhtxx.c)
    set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)
endif()

if (${GPIO_DS182X})
    FILE(GLOB SOURCES ./ds182x.c)
    set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)
endif()

if (${GPIO_MAX7219})
    FILE(GLOB SOURCES ./max7219.c)
    set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/max7219.cmake)
endif()

# Add this directory to the include directory.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "RTOS_INCLUDES" FORCE)