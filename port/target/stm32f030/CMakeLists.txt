# Add subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/board)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/generic)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/io)

# Update link flags to include linker file.
STRING(APPEND CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs -T${CMAKE_CURRENT_SOURCE_DIR}/stm32f030f4p6.ld")
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} CACHE STRING "" FORCE)

# Add a configuration option for target platform.
setup_option(TARGET_PLATFORM "PLAT_STM32F030F4P6")
setup_option_def(TARGET_PLATFORM "PLAT_STM32F030F4P6" MACRO "Target platform.")
setup_option_hide(TARGET_PLATFORM)

# Set heap size for this platform.
setup_option_def(TARGET_HEAP_SIZE "2048" MACRO "Target heap size.")