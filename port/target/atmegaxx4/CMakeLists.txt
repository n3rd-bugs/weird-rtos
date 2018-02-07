# Inlcude platform definitions.
include(${CMAKE_CURRENT_SOURCE_DIR}/atmegaxx4.cmake)

# Add subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/board)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/io)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils)

# If we are targeting  
if (TGT_PLATFORM STREQUAL atmega1284)
    # Update link flags to include linker file.
    set(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/atmega1284p.ld ${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
endif ()

# Add a configuration option for target platform.
setup_option(TARGET_PLATFORM "PLAT_ATMEGAXX4")
setup_option_def(TARGET_PLATFORM "PLAT_ATMEGAXX4" MACRO "Target platform.")
setup_option_hide(TARGET_PLATFORM)

# Set heap size for this platform.
setup_option_def(TARGET_HEAP_SIZE "0" MACRO "Target heap size.")