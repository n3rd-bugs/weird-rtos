# Setup configuration options.
setup_option_def(STM_I2C_BUSY_YIELD OFF DEFINE "If we need to yeild the calling task while we busy wait for an event on I2C bus." CONFIG_FILE "i2c_stm32_config")
setup_option_def(STM_I2C_INT_TIMEOUT 50 INT "Timeout in miliseconds to wait for a message to process." CONFIG_FILE "i2c_stm32_config")
setup_option_def(STM_I2C_INT_MODE OFF DEFINE "If we need to run I2C in interrupt mode." CONFIG_FILE "i2c_stm32_config")