# This function will configure an option.
function (setup_option configuration value type)
    # Set this option as provided.
    set(${configuration} ${value} CACHE ${type} "")
endfunction ()

# This function will setup a RTOS configuration.
function (setup_option_def configuration default_value type description)
    # Setup a configuration option.
    set(${configuration} ${default_value} CACHE ${type} ${description})
    
    # If option is enabled.
    if (${configuration})
        # Add this to RTOS definition list.
        list(APPEND RTOS_DEFS -D${configuration})
        SET(RTOS_DEFS ${RTOS_DEFS} CACHE INTERNAL "RTOS_DEFS" FORCE)
    endif ()
endfunction ()
