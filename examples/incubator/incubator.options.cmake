# Include helpers.
include(${RTOS_ROOT}/cmake/modules/helper.cmake)

# Setup target configuration.
setup_option(TGT_PLATFORM atmega644p)
setup_option(F_CPU 16000000UL)

# Initialize RTOS configurations.
setup_option(CONFIG_ADC ON)
setup_option(CONFIG_FS ON)
setup_option(CONFIG_SERIAL ON)

# Update the number of ticks per second to 1000.
setup_option(SOFT_TICKS_PER_SEC 1000)

# Setup task options.
setup_option(TASK_STATS ON)
setup_option(TASK_USAGE ON)
setup_option(TASK_USAGE_RETAIN ON)

# Setup AVR configurations.
setup_option(ADC_AVR_PRESCALE ADC_AVR_DIV_128)
setup_option(ADC_AVR_TIMER_PRESCALE TIMER0_AVR_DIV_1024)
setup_option(ADC_AVR_REF ADC_AVR_REF_INT_1_1)
setup_option(SERIAL_INTERRUPT_MODE ON)