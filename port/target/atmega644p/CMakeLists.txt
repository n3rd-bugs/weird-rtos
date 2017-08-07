# Add subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/board)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/io)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils)

# Update link flags to include linker file.
STRING(APPEND CMAKE_EXE_LINKER_FLAGS " -T${CMAKE_CURRENT_SOURCE_DIR}/atmega1284p.ld")
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} CACHE STRING "" FORCE)

# Add a configuration option for target platform.
setup_option_def(PLAT_TARGET "PLAT_ATMEGA644P" MACRO "Target platform.")
setup_option_hide(PLAT_TARGET)