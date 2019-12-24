# This function will setup an RTOS configuration.
function (setup_option_def configuration default_value type description)
    # Get the value function if available.
    set(options VALUE_FUN VALUE_LIST CONFIG_FILE)
    cmake_parse_arguments(setup_option_def "" "${options}" "" ${ARGN})

    # Pick configuration type.
    if (${type} STREQUAL "BOOL")
        set(cache_type BOOL)
    elseif (${type} STREQUAL "DEFINE")
        set(cache_type BOOL)
    else ()
        set(cache_type STRING)
    endif ()

    # Setup a configuration option.
    set(${configuration} ${default_value} CACHE ${cache_type} "")

    # Force to add option description.
    set(${configuration} ${${configuration}} CACHE ${cache_type} ${description} FORCE)

    # If a value function was provided.
    if (setup_option_def_VALUE_FUN)
        # Translate the value.
        include(${${setup_option_def_VALUE_FUN}})
        value_map(${${configuration}} configuration_value)
    else ()
        # Use the given value as it is.
        set(configuration_value ${${configuration}} CACHE INTERNAL "" FORCE)
    endif ()

    # If a value list was provided.
    if (setup_option_def_VALUE_LIST)
        # Set possible options for this configuration.
        set_property(CACHE ${configuration} PROPERTY STRINGS ${${setup_option_def_VALUE_LIST}})
        if(NOT ${${configuration}} IN_LIST ${setup_option_def_VALUE_LIST})
            message(FATAL_ERROR "${configuration} must be one of ${${setup_option_def_VALUE_LIST}}")
        endif()
    endif ()

    # If we have a destination configuration file.
    if (setup_option_def_CONFIG_FILE)
        list(APPEND RTOS_DEFS_FILES "${setup_option_def_CONFIG_FILE}")
    else ()
        list(APPEND RTOS_DEFS_FILES "config_cmake")
    endif ()
    set(RTOS_DEFS_FILES ${RTOS_DEFS_FILES} CACHE INTERNAL "RTOS_DEFS_FILES" FORCE)

    # If this is a define option.
    if (${type} STREQUAL "DEFINE")
        # If option is enabled.
        if (${configuration})
            # Add this to RTOS configuration list.
            set(configuration_output "#define ${configuration}")
        else ()
            # Un-define this configuration.
            set(configuration_output "#undef ${configuration}")
        endif ()
    # If this is a boolean option.
    elseif (${type} STREQUAL "BOOL")
        # If option is enabled.
        if (${configuration})
            # Add this to RTOS configuration list as TRUE.
            set(configuration_output "#define ${configuration} TRUE")
        else ()
            # Add this to RTOS configuration list as FALSE.
            set(configuration_output "#define ${configuration} FALSE")
        endif ()
    elseif (${type} STREQUAL "STRING")
        # Add this to RTOS configuration list with a string value.
        set(configuration_output "#define ${configuration} \"${configuration_value}\"")
    elseif (${type} STREQUAL "CHAR")
        # Add this to RTOS configuration list with a character value.
        set(configuration_output "#define ${configuration} \'${configuration_value}\'")
    elseif (${type} STREQUAL "INT")
        # Add this to RTOS configuration list with an integral value.
        set(configuration_output "#define ${configuration} \(${configuration_value}\)")
    elseif (${type} STREQUAL "UINT32")
        # Add this to RTOS configuration list with an integral value.
        set(configuration_output "#define ${configuration} \((uint32_t)${configuration_value}\)")
    elseif (${type} STREQUAL "MACRO")
        # Add this to RTOS configuration list as a MACRO value.
        set(configuration_output "#define ${configuration} \(${configuration_value}\)")
    elseif (${type} STREQUAL "MACRO_OPEN")
        # Add this to RTOS configuration list as a MACRO open value.
        set(configuration_output "#define ${configuration} ${configuration_value}")
    else ()
        # Option type is not supported.
        message(FATAL_ERROR "Unsupported option type." )
    endif ()

    # Add this to RTOS configuration.
    list(APPEND RTOS_DEFS "${configuration_output}")
    set(RTOS_DEFS ${RTOS_DEFS} CACHE INTERNAL "RTOS_DEFS" FORCE)
endfunction ()

# This function will configure an option.
function (setup_option configuration value)
    # Set this option as provided.
    set(${configuration} ${value} CACHE INTERNAL "" FORCE)
endfunction ()

# This function will make an option hidden.
function (setup_option_hide configuration)
    # Force to to hide this option.
    set(${configuration} ${${configuration}} CACHE INTERNAL "" FORCE)
endfunction ()

# This function will test if all the members in the given configuration do exist in the given list.
function (setup_option_list_values configuration values delimiter)
    # First remove all the spaces.
    string(REPLACE " " "" configuration_list ${${configuration}})

    # Now divide the string on the given delimiter.
    string(REPLACE ${delimiter} ";" configuration_list ${configuration_list})

    # Verify all the items in the configuration does exist in the given list.
    foreach(list_item ${configuration_list})
        if(NOT ${list_item} IN_LIST ${values})
            message(FATAL_ERROR "${configuration} must only contain these ${${values}}")
        endif()
    endforeach()
endfunction ()

# This function will generate configuration file.
function (generate_configuration configuration_list file_path file_list)
    # Get the number of configurations.
    list(LENGTH ${configuration_list} count)
    math(EXPR count "${count}-1")

    # Get the number of files.
    list(LENGTH ${file_list} count)

    # Clear file list.
    unset(CONFIG_FILE_LIST CACHE)
    unset(CONFIG_FILENAME_LIST CACHE)

    # If we have atleast one configuration.
    if (${count} GREATER 0)
        # Start from 0 index.
        math(EXPR count "${count}-1")

        # Parse all the configurations and initialize file data.
        foreach (i RANGE ${count})
            # Pick the configuration and destination configuration file.
            list(GET ${configuration_list} ${i} config)
            list(GET ${file_list} ${i} file_name)

            # If we are creating a new file.
            if (NOT DEFINED CONFIG_FILE_${file_name})
                # Add file header.
                set(CONFIG_FILE_${file_name} "/* This file is auto-generated. Do not edit! */
/*
 * ${file_name}.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _${file_name}_h_
#define _${file_name}_h_
")
                # Save file data and the actual file name so we can later write it.
                list(APPEND CONFIG_FILE_LIST CONFIG_FILE_${file_name})
                list(APPEND CONFIG_FILENAME_LIST ${file_name})
            endif ()

            # If we need to add this configuration.
            if (config)
                # Add this configuration to the file data.
                set(CONFIG_FILE_${file_name} "${CONFIG_FILE_${file_name}}\n${config}" CACHE INTERNAL "" FORCE)
            endif ()
        endforeach ()
    endif ()

    # Get the number of configuration files.
    list(LENGTH CONFIG_FILE_LIST count)

    # If we have atleast one file.
    if (${count} GREATER 0)
        # Start from 0 index.
        math(EXPR count "${count}-1")

        # Add each configuration option.
        foreach (i RANGE ${count})
            # Get the file data and the corresponding file name.
            list(GET CONFIG_FILE_LIST ${i} config_file)
            list(GET CONFIG_FILENAME_LIST ${i} file_name)

            # Append file footer.
            set(config_file "${${config_file}}\n\n#endif /* _${file_name}_h_ */\n")

            # Test if configuration file already exists.
            if (EXISTS "${file_path}/${file_name}.h")
                # Read the existing configuration file.
                file(READ "${file_path}/${file_name}.h" file_data)
            endif ()

            # See if we really need to update this configuration file.
            if ((NOT EXISTS "${file_path}/${file_name}.h") OR (NOT ${file_data} STREQUAL ${config_file}))
                # Write this configuration file.
                file(WRITE "${file_path}/${file_name}.h" "${config_file}")
            endif ()

            # Again get the file data.
            list(GET CONFIG_FILE_LIST ${i} config_file)

            # Unset the file data so we don't add it to cache list.
            unset(${config_file} CACHE)
        endforeach ()
    endif ()
endfunction ()
