# Add required platform folders according to target.
if (${TGT_CPU} STREQUAL "atmega644")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/kernel/avr)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/atmegaxx4)
elseif (${TGT_CPU} STREQUAL "atmega1284")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/kernel/avr)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/atmegaxx4)
elseif (${TGT_CPU} STREQUAL "cortex-m4")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/kernel/cortex-m4)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/stm32f407discovery)
endif ()

# Add tools directory.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/toolset/${TGT_TOOL})

# Add kernel directory to the include directory.
LIST(APPEND RTOS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/kernel)

# Update the project variables.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} CACHE INTERNAL "RTOS_INCLUDES" FORCE)