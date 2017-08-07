# Add subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/board)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/generic)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/io)

# Update link flags to include linker file.
STRING(APPEND CMAKE_EXE_LINKER_FLAGS " -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F407.ld")
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} CACHE STRING "" FORCE)
