# Add required platform folders according to target.
set(atmegaxx4 atmega644 atmega644p atmega644a atmega644pa atmega1284 atmega1284p CACHE INTERNAL "" FORCE)
if (${TGT_PLATFORM} IN_LIST atmegaxx4)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/atmegaxx4)
elseif (${TGT_PLATFORM} STREQUAL "stm32f030f4p6")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/stm32f030)
elseif (${TGT_PLATFORM} STREQUAL "stm32f103c8t6")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/stm32f103)
elseif (${TGT_PLATFORM} STREQUAL "stm32f407vgt6")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/stm32f407)
elseif (${TGT_PLATFORM} STREQUAL "stm32f411ceu6")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/stm32f411)
endif ()

# Add CPU directory.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/kernel/${TGT_CPU})

# Add tools directory.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/toolset/${TGT_TOOL})

# Add kernel directory to the include directory.
LIST(APPEND RTOS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/kernel)

# Update the project variables.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} CACHE INTERNAL "RTOS_INCLUDES" FORCE)