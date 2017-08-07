# Add subdirectories.
if (${CONFIG_TFTPS})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tftp)
endif ()
if (${CONFIG_WEIRD_VIEW})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/weird_view)
endif ()