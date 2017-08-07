# Include helpers.
include(${RTOS_ROOT}/cmake/modules/helper.cmake)

# Initialize RTOS configurations.
setup_option(CONFIG_FS ON)
setup_option(CONFIG_NET ON)
setup_option(CONFIG_I2C ON)
setup_option(CONFIG_PCF8574 ON)
setup_option(CONFIG_SPI ON)
setup_option(CONFIG_MMC OFF)
setup_option(CONFIG_ADC ON)
setup_option(CONFIG_LCD_AN ON)
setup_option(CONFIG_LCD_PCF8574 ON)
setup_option(CONFIG_ETHERNET ON)
setup_option(CONFIG_WEIRD_VIEW ON)

# Setup IDLE task options.
setup_option(IDLE_WORK_MAX 1)
setup_option(IDLE_TASK_STACK_SIZE 196)

# Update the number of ticks per second to 10.
setup_option(SOFT_TICKS_PER_SEC 10)

# Setup task options.
setup_option(CONFIG_TASK_STATS ON)
setup_option(CONFIG_TASK_USAGE ON)

# Setup target configuration.
setup_option(PLATFORM atmega644)
setup_option(F_CPU 20000000UL)
