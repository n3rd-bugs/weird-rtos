# Configure API components.
setup_option_def(CONFIG_WEIRD_VIEW OFF DEFINE "Enable weird-view IOT framework.")
setup_option_def(CONFIG_TFTPS OFF DEFINE "Enable TFTP server support.")

# Add subdirectories.
if (${CONFIG_TFTPS})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tftp)
endif ()
if (${CONFIG_WEIRD_VIEW})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/weird_view)
endif ()