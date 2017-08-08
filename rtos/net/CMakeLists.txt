# Make a list of all the files in this folder and append them to the ${RTOS_SOURCES}.
FILE(GLOB SOURCES ./*.c)
set(RTOS_SOURCES ${RTOS_SOURCES} ${SOURCES} CACHE INTERNAL "RTOS_SOURCES" FORCE)

# Add this directory to the include directory.
SET(RTOS_INCLUDES ${RTOS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "RTOS_INCLUDES" FORCE)

# Inlcude configuration options.
include(${CMAKE_CURRENT_SOURCE_DIR}/net_condition.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/net_dhcp_client.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/net_icmp.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/net_ipv4.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/net_route.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/net_tcp.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/net_udp.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/net.cmake)