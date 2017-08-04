# Load default AVR configurations.
set(MCU atmega644 CACHE STRING "Target MCU.")
set(F_CPU 20000000UL CACHE STRING "Target frequency.")

# Find required tool-sets.
find_program(AVR_CC avr-gcc)
find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_OBJDUMP avr-objdump)
find_program(AVR_SIZE avr-size)

# Set default compiler.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER ${AVR_CC})

# Enable RESPONSE files to resolve linker errors.
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS ON)

# Load default flags.
set(AVR_C_FLAGS "-Wall -Os -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -Wextra -mrelax -Wstrict-prototypes")
set(AVR_MCU "-mmcu=${MCU}")
set(AVR_FRQ "-DF_CPU=${F_CPU}")
set(AVR_LINK_FLAGS "-Wl,--gc-sections -lm")

# Set c and link flags.
set(CMAKE_C_FLAGS "${AVR_MCU} ${AVR_FRQ} ${AVR_C_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${AVR_MCU} ${AVR_LINK_FLAGS}" CACHE STRING "" FORCE)

# This function will setup a target for AVR.
function (setup_target target_name sources)
    # Add CMAKE_BUILD to disable manual configurations.
    add_definitions(-DCMAKE_BUILD)

    # Add an executable target.
    add_executable(${target_name} ${RTOS_LINK_SOURCES} ${${sources}})
    target_link_libraries(${target_name} ${RTOS_LIB})
    target_include_directories(${target_name} PUBLIC ${RTOS_INCLUDES})
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME ${target_name}.elf LINK_FLAGS "-Wl,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.map")
    
    # Add target to generate a HEX file for this build.
    add_custom_target(${target_name}.hex ALL ${AVR_OBJCOPY} -R .eeprom -R .fuse -R .lock -R .signature -O ihex "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.elf" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.hex" DEPENDS ${target_name})
    
    # Add a target to create ASM listing.
    add_custom_target(${target_name}.lss ALL ${AVR_OBJDUMP} -h -S "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.elf" > "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.lss" DEPENDS ${target_name})
    
    # Add a target to provide display analysis.
    add_custom_target(${target_name}.size ALL ${AVR_SIZE} --format=avr --mcu=${MCU} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.elf" DEPENDS ${target_name})
endfunction ()