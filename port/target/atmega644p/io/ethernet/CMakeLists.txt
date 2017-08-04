# Add subdirectories.
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/enc28j60)

# Make a list of all the files in this folder and append them to the ${RTOS_SOURCES}.
FILE(GLOB SOURCES ./*.c)
set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)

# Add this directory to the include directory.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "RTOS_INCLUDES" FORCE)
