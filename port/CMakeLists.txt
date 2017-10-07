# Add required platform folders according to target.
if (${TGT_PLATFORM} STREQUAL "atmega644")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/atmegaxx4)
elseif (${TGT_PLATFORM} STREQUAL "atmega1284")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/atmegaxx4)
elseif (${TGT_PLATFORM} STREQUAL "stm32f407discovery")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/stm32f407)
elseif (${TGT_PLATFORM} STREQUAL "stm32f103c8t6")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/target/stm32f103)
endif ()

# Add CPU directory.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/kernel/${TGT_CPU})

# Add tools directory.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/toolset/${TGT_TOOL})

# Add kernel directory to the include directory.
LIST(APPEND RTOS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/kernel)

# Update the project variables.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} CACHE INTERNAL "RTOS_INCLUDES" FORCE)