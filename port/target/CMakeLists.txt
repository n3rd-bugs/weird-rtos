# Add subdirectories.
if (${PLATFORM} STREQUAL "atmega644")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/atmega644p)
elseif (${PLATFORM} STREQUAL "stm32f407discovery")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/stm32f407discovery)
endif ()
