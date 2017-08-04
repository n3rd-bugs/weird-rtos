# This function will configure an option.
function (setup_option configuration value type)    
    # Pick configuration type.
    if (${type} STREQUAL "BOOL")
        set(cache_type BOOL)
    else ()
        set(cache_type STRING)
    endif ()
    # Set this option as provided.
    set(${configuration} ${value} CACHE ${cache_type} "")
endfunction ()

# This function will setup an RTOS configuration.
function (setup_option_def configuration default_value type description)
    # Pick configuration type.
    if (${type} STREQUAL "BOOL")
        set(cache_type BOOL)
    else ()
        set(cache_type STRING)
    endif ()
    
    # Setup a configuration option.
    set(${configuration} ${default_value} CACHE ${cache_type} "")
    
    # Force to add option description.
    set(${configuration} ${${configuration}} CACHE ${cache_type} ${description} FORCE)
    
    # If this is a define option.
    if (${type} STREQUAL "BOOL")
        # If option is enabled.
        if (${configuration})
            # Add this to RTOS configuration list.
            SET(RTOS_DEFS "${RTOS_DEFS}#define ${configuration}\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
        endif ()
    elseif (${type} STREQUAL "STRING")
        # Add this to RTOS configuration list with a string value.
        SET(RTOS_DEFS "${RTOS_DEFS}#define ${configuration} \"${${configuration}}\"\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
    elseif (${type} STREQUAL "CHAR")
        # Add this to RTOS configuration list with a character value.
        SET(RTOS_DEFS "${RTOS_DEFS}#define ${configuration} \'${${configuration}}\'\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
    elseif (${type} STREQUAL "INT")
        # Add this to RTOS configuration list with an integral value.
        SET(RTOS_DEFS "${RTOS_DEFS}#define ${configuration} ${${configuration}}\n" CACHE INTERNAL "RTOS_DEFS" FORCE)
    else ()
        # Option type is not supported.
        message(FATAL_ERROR "Unsupported option type." )
    endif ()
endfunction ()

# This function will generate configuration file.
function (generate_configuration configuration file_path file_name)
    # Add file header.
    set(CONFIG_FILE "/* This file is auto-generated. Do not edit! */
/*
 * ${file_name}
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
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
