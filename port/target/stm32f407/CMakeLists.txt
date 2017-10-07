# Add subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/board)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/generic)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/io)

# Update link flags to include linker file.
STRING(APPEND CMAKE_EXE_LINKER_FLAGS " -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F407.ld")
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} CACHE STRING "" FORCE)

# Add a configuration option for target platform.
setup_option(TARGET_PLATFORM "PLAT_STM32F407_DISC")
setup_option_def(TARGET_PLATFORM "PLAT_STM32F407_DISC" MACRO "Target platform.")
setup_option_hide(TARGET_PLATFORM)