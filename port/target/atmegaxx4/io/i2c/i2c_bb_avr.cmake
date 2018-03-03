# Setup configuration options.
setup_option_def(AVR_SLOW_I2C ON DEFINE "Use slow I2C mode." CONFIG_FILE "i2c_bb_avr_config")
setup_option_def(AVR_I2C_DELAY 1 INT "Time in microseconds to delay the SCL transition." CONFIG_FILE "i2c_bb_avr_config")