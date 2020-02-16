# Include helpers.
include(${RTOS_ROOT}/cmake/modules/helper.cmake)

# Setup driver configurations.
setup_option(IO_I2C ON)
setup_option(IO_OLED ON)

# Setup task options.
setup_option(TASK_STATS ON)
setup_option(TASK_USAGE ON)

# Setup I2C configuration.
setup_option(AVR_SLOW_I2C OFF)