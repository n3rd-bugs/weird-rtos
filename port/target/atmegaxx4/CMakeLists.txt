# Inlcude platform definitions.
include(${CMAKE_CURRENT_SOURCE_DIR}/atmegaxx4.cmake)

# Add subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/board)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/io)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils)

# If we are targeting  
if (PLATFORM STREQUAL atmega1284)
    # Update link flags to include linker file.
    set(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/atmega1284p.ld ${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
    message("Using ${CMAKE_CURRENT_SOURCE_DIR}/atmega1284p.ld to link the target")
endif ()

# Add a configuration option for target platform.
setup_option_def(PLAT_TARGET "PLAT_ATMEGAXX4" MACRO "Target platform.")
setup_option_hide(PLAT_TARGET)