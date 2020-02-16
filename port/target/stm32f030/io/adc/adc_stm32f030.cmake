# Setup configuration options.
set(ADC_STM32_SAMPLE_TIME_VALUES 0 1 2 3 4 5 6 7)
setup_option_def(ADC_STM32_SAMPLE_TIME 4 INT "ADC sample time to be used." VALUE_LIST ADC_STM32_SAMPLE_TIME_VALUES CONFIG_FILE "adc_stm32_config")
setup_option_def(ADC_STM32_PERIODIC_INTERVAL 0 INT "Periodic interval in (ms) at which to trigger ADC." CONFIG_FILE "adc_stm32_config")