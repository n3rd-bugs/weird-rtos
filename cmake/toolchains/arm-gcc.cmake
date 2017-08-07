# Load default configurations.
set(PLATFORM stm32f407discovery CACHE STRING "")
set(PLATFORM ${PLATFORM} CACHE STRING "Target platform.")
set(TGT_TOOL "gcc-arm" CACHE STRING "Target Tools." FORCE)

# Find required tool-sets.
find_program(ARM_CC arm-none-eabi-gcc)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy)
find_program(ARM_OBJDUMP arm-none-eabi-objdump)
find_program(ARM_SIZE arm-none-eabi-size)

# Set default compiler.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER ${ARM_CC})

# Enable RESPONSE files to resolve linker errors.
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS ON)

# Load default flags.
set(ARM_C_FLAGS "-Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wpadded -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g3 -std=gnu11" CACHE STRING "C flags.")
set(ARM_LINK_FLAGS "--specs=nosys.specs -Xlinker --gc-sections " CACHE STRING "LD flags.")

# Select the target CPU.
if (${PLATFORM} STREQUAL "stm32f407discovery")
    # We have a cortex-M4.
    set(TGT_CPU "cortex-m4" CACHE STRING "Target CPU.")
    
    # Update c-flags.
    set(ARM_C_FLAGS "-mcpu=${TGT_CPU} -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 ${ARM_C_FLAGS}")
else()
    message(FATAL_ERROR "Unsupported device ${PLATFORM}.")
endif ()

# Set c and link flags.
set(CMAKE_C_FLAGS "${ARM_C_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${ARM_LINK_FLAGS}" CACHE STRING "" FORCE)

# This function will setup a target for ARM.
function (setup_target target_name sources)
    # Add CMAKE_BUILD to disable manual configurations.
    add_definitions(-DCMAKE_BUILD)

    # Add an executable target.
    add_executable(${target_name} ${RTOS_LINK_SOURCES} ${${sources}})
    target_link_libraries(${target_name} ${RTOS_LIB})
    target_include_directories(${target_name} PUBLIC ${RTOS_INCLUDES})
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME ${target_name}.elf LINK_FLAGS "-Wl,-Map,${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.map")
    
    # Add target to generate a HEX file for this build.
    add_custom_target(${target_name}.hex ALL ${ARM_OBJCOPY} -O ihex "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.elf" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.hex" DEPENDS ${target_name})
    
    # Add a target to create ASM listing.
    add_custom_target(${target_name}.lss ALL ${ARM_OBJDUMP} -h -S "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.elf" > "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.lss" DEPENDS ${target_name})
    
    # Add a target to display size analysis.
    add_custom_target(${target_name}.size ALL ${ARM_SIZE} --format=berkeley "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target_name}.elf" DEPENDS ${target_name})
endfunction ()