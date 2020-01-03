# Setup configuration options.
setup_option_def(DHTXX_STM32_DATA_PORT GPIOB INT "GPIO port to be used for data." VALUE_FUN stm32f030_regmap VALUE_LIST stm32f030_port_values CONFIG_FILE "dhtxx_stm32_config")
setup_option_def(DHTXX_STM32_DATA_PIN 1 INT "GPIO pin number to be used for data." VALUE_LIST stm32f030_pinnum_values CONFIG_FILE "dhtxx_stm32_config")