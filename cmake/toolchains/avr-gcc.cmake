# Load default AVR configurations.
set(TGT_PLATFORM atmega644 CACHE STRING "Target platform.")
set_property(CACHE TGT_PLATFORM PROPERTY STRINGS "atmega644" "atmega1284")
set(TGT_CPU "avr" CACHE INTERNAL "Target CPU." FORCE)
set(TGT_TOOL "gcc-avr" CACHE STRING "Target Tools.")
set(F_CPU 20000000UL CACHE STRING "Target frequency.")

# Find required tool-sets.
find_program(AVR_CC avr-gcc)
find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_OBJDUMP avr-objdump)
find_program(AVR_SIZE avr-size)
find_program(AVR_DUDE avrdude)

# Enable RESPONSE files to resolve linker errors.
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS ON)

# Load default flags.
set(AVR_C_FLAGS "-Os -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -Wextra -mrelax -Wall -Wstrict-prototypes" CACHE STRING "C flags.")
set(AVR_MCU_FLAGS "-mmcu=${TGT_PLATFORM}" CACHE INTERNAL "" FORCE)
set(AVR_FRQ_FLAGS "-DF_CPU=${F_CPU}" CACHE INTERNAL "" FORCE)
set(AVR_LINK_FLAGS "-Wl,--gc-sections ${AVR_MCU_FLAGS}" CACHE STRING "LD flags." FORCE)

# Setup c compiler.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER ${AVR_CC})
set(CMAKE_C_FLAGS "${AVR_MCU_FLAGS} ${AVR_FRQ_FLAGS} ${AVR_C_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${AVR_LINK_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_C_LINK_EXECUTABLE "\"${AVR_CC}\" <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>" CACHE STRING "" FORCE)

# This function will setup a target for AVR.
function (setup_target target_name sources)
    # Add CMAKE_BUILD to disable manual configurations.
    add_definitions(-DCMAKE_BUILD)

    # Add an executable target.
    add_executable(${target_name} ${RTOS_LINK_SOURCES} ${${sources}})
    target_link_libraries(${target_name} ${RTOS_LIB} m)
    target_include_directories(${target_name} PUBLIC ${RTOS_INCLUDES})
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME ${target_name}.elf LINK_FLAGS "-Wl,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.map")

    # Add target to generate a HEX file for this build.
    add_custom_target(${target_name}.hex ALL ${AVR_OBJCOPY} -R .eeprom -R .fuse -R .lock -R .signature -O ihex "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.elf" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.hex" DEPENDS ${target_name})

    # Add a target to create ASM listing.
    add_custom_target(${target_name}.lss ALL ${AVR_OBJDUMP} -h -S "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.elf" > "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.lss" DEPENDS ${target_name})

    # Add a target to display size analysis.
    add_custom_target(${target_name}.size ALL ${AVR_SIZE} --format=avr --mcu=${TGT_PLATFORM} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.elf" DEPENDS ${target_name})

    # If we have AVR dude configuration.
    if (${TGT_PLATFORM}_DUDE_MCU)
        # Add a target for AVR dude.
        add_custom_target(${target_name}.dude ${AVR_DUDE} -p${${TGT_PLATFORM}_DUDE_MCU} -c${${TGT_PLATFORM}_DUDE_DRIVER} -P${${TGT_PLATFORM}_DUDE_SER} -b${${TGT_PLATFORM}_DUDE_BOUD} -D -Uflash:w:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.hex:a DEPENDS ${target_name}.hex)
    endif ()
endfunction ()