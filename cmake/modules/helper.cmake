# This function will setup an RTOS configuration.
function (setup_option_def configuration default_value type description)
    # Get the value funtion if available.
    set(options VALUE_FUN VALUE_LIST)
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
    
    # If a value funstion was provided.
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
    
    # If this is a define option.
    if (${type} STREQUAL "DEFINE")
        # If option is enabled.
        if (${configuration})
            # Add this to RTOS configuration list.
            set(RTOS_DEFS "${RTOS_DEFS}#define ${configuration}\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
        endif ()
    # If this is a boolean option.
    elseif (${type} STREQUAL "BOOL")
        # If option is enabled.
        if (${configuration})
            # Add this to RTOS configuration list as TRUE.
            set(RTOS_DEFS "${RTOS_DEFS}#define ${configuration} TRUE\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
        else ()
            # Add this to RTOS configuration list as FALSE.
            set(RTOS_DEFS "${RTOS_DEFS}#define ${configuration} FALSE\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
        endif ()
    elseif (${type} STREQUAL "STRING")
        # Add this to RTOS configuration list with a string value.
        set(RTOS_DEFS "${RTOS_DEFS}#define ${configuration} \"${configuration_value}\"\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
    elseif (${type} STREQUAL "CHAR")
        # Add this to RTOS configuration list with a character value.
        set(RTOS_DEFS "${RTOS_DEFS}#define ${configuration} \'${configuration_value}\'\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
    elseif (${type} STREQUAL "INT")
        # Add this to RTOS configuration list with an integral value.
        set(RTOS_DEFS "${RTOS_DEFS}#define ${configuration} \(${configuration_value}\)\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
    elseif (${type} STREQUAL "MACRO")
        # Add this to RTOS configuration list as a MACRO value.
        set(RTOS_DEFS "${RTOS_DEFS}#define ${configuration} \(${configuration_value}\)\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
    elseif (${type} STREQUAL "MACRO_OPEN")
        # Add this to RTOS configuration list as a MACRO open value.
        set(RTOS_DEFS "${RTOS_DEFS}#define ${configuration} ${configuration_value}\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
    else ()
        # Option type is not supported.
        message(FATAL_ERROR "Unsupported option type." )
    endif ()
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
function (generate_configuration configuration file_path file_name)
    # Add file header.
    set(CONFIG_FILE "/* This file is auto-generated. Do not edit! */
/*
 * ${file_name}
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
#ifndef _CONFIG_CMAKE_H_
#define _CONFIG_CMAKE_H_

")
                    
    # Add each configuration option.
    foreach (config ${configuration})
        set(CONFIG_FILE "${CONFIG_FILE}${config}\n")
    endforeach ()
    
    # Append file footer.
    set(CONFIG_FILE "${CONFIG_FILE}#endif /* _CONFIG_CMAKE_H_ */\n")
    
    # Write configuration file.
    file(WRITE "${file_path}/${file_name}" "${CONFIG_FILE}")
endfunction ()
