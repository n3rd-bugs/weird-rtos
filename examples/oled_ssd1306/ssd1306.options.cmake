# Include helpers.
include(${RTOS_ROOT}/cmake/modules/helper.cmake)

# Setup task options.
setup_option(CONFIG_TASK_STATS ON)
setup_option(CONFIG_TASK_USAGE ON)

# Setup I2C configuration.
setup_option(AVR_SLOW_I2C OFF)