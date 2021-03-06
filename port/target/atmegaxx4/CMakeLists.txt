# Inlcude platform definitions.
include(${CMAKE_CURRENT_SOURCE_DIR}/atmegaxx4.cmake)

# Add subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/board)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/io)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils)

# Setup linker flags.
if (TGT_PLATFORM STREQUAL atmega1284p)
    # Update link flags to include linker file.
    set(CMAKE_EXE_LINKER_FLAGS "-T\"${CMAKE_CURRENT_SOURCE_DIR}/atmega1284p.ld\" ${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
elseif (TGT_PLATFORM STREQUAL atmega644p)
    # Update link flags to include linker file.
    set(CMAKE_EXE_LINKER_FLAGS "-T\"${CMAKE_CURRENT_SOURCE_DIR}/atmega644p.ld\" ${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
endif ()

# Add a configuration option for target platform.
setup_option(TARGET_PLATFORM "PLAT_ATMEGAXX4")
setup_option_def(TARGET_PLATFORM "PLAT_ATMEGAXX4" MACRO "Target platform.")
setup_option_hide(TARGET_PLATFORM)

# Add a configuration option for target MCU
setup_option_def(AVR_MCU_${TGT_PLATFORM} ON DEFINE "Target MCU.")
setup_option_hide(AVR_MCU_${TGT_PLATFORM})

# Set heap size for this platform.
setup_option_def(TARGET_HEAP_SIZE "0" MACRO "Target heap size.")

# Configure bootloader for this platform
setup_option_def(CONFIG_BOOTLOAD OFF DEFINE "Enable bootloader support.")

# Setup AVR dude options.
if (TGT_PLATFORM STREQUAL atmega1284)
    setup_option_def(${TGT_PLATFORM}_DUDE_MCU "m1284" STRING "AVR dude MCU." CONFIG_FILE "unused")
endif ()
if (TGT_PLATFORM STREQUAL atmega1284p)
    setup_option_def(${TGT_PLATFORM}_DUDE_MCU "m1284p" STRING "AVR dude MCU." CONFIG_FILE "unused")
endif ()
if (TGT_PLATFORM STREQUAL atmega644)
    setup_option_def(${TGT_PLATFORM}_DUDE_MCU "m644" STRING "AVR dude MCU." CONFIG_FILE "unused")
endif ()
if (TGT_PLATFORM STREQUAL atmega644p)
    setup_option_def(${TGT_PLATFORM}_DUDE_MCU "m644p" STRING "AVR dude MCU." CONFIG_FILE "unused")
endif ()

# If we have an AVR for which avrdude is supported setup rest of the options.
if (${TGT_PLATFORM}_DUDE_MCU)
    set(AVR_DUDE_DRIVERS stk500v1 arduino)
    setup_option_def(${TGT_PLATFORM}_DUDE_DRIVER "stk500v1" STRING "AVR dude driver." VALUE_LIST AVR_DUDE_DRIVERS CONFIG_FILE "unused")
    setup_option_def(${TGT_PLATFORM}_DUDE_SER "COM1" STRING "AVR dude serial port." CONFIG_FILE "unused")
    setup_option_def(${TGT_PLATFORM}_DUDE_BOUD "115200" STRING "AVR dude boudrate." CONFIG_FILE "unused")
endif ()