# Include helpers.
include(${RTOS_ROOT}/cmake/modules/helper.cmake)

# Initialize RTOS configurations.
setup_option(CONFIG_FS ON BOOL)
setup_option(CONFIG_NET ON BOOL)
setup_option(CONFIG_I2C ON BOOL)
setup_option(CONFIG_PCF8574 ON BOOL)
setup_option(CONFIG_SPI ON BOOL)
setup_option(CONFIG_MMC OFF BOOL)
setup_option(CONFIG_ADC ON BOOL)
setup_option(CONFIG_LCD_AN ON BOOL)
setup_option(CONFIG_LCD_PCF8574 ON BOOL)
setup_option(CONFIG_ETHERNET ON BOOL)
setup_option(CONFIG_WEIRD_VIEW ON BOOL)

# Setup IDLE task options.
setup_option(IDLE_WORK_MAX 1 INT)
setup_option(IDLE_TASK_STACK_SIZE 196 INT)

# Setup task options.
setup_option(CONFIG_TASK_STATS ON BOOL)
setup_option(CONFIG_TASK_USAGE ON BOOL)

# Setup target configuration.
setup_option(PLATFORM atmega644 STRING)
setup_option(F_CPU 20000000UL STRING)
