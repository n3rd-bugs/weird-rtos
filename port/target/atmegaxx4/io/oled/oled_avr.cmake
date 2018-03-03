# Setup configuration options.
setup_option_def(OLED_AVR_I2C_ADDRESS 0x3C INT "I2C address for SSD1306 controller." CONFIG_FILE "oled_avr_config")
setup_option_def(OLED_AVR_PIN_SCL PINB INT "SCL PIN register for I2C of SSD1306 controller." VALUE_FUN atmega_regmap VALUE_LIST atmega_pin_values CONFIG_FILE "oled_avr_config")
setup_option_def(OLED_AVR_PIN_SDA PINB INT "SDA PIN register for I2C of SSD1306 controller." VALUE_FUN atmega_regmap VALUE_LIST atmega_pin_values CONFIG_FILE "oled_avr_config")
setup_option_def(OLED_AVR_DDR_SCL DDRB INT "SCL DDR register for I2C of SSD1306 controller." VALUE_FUN atmega_regmap VALUE_LIST atmega_ddr_values CONFIG_FILE "oled_avr_config")
setup_option_def(OLED_AVR_DDR_SDA DDRB INT "SDA DDR register for I2C of SSD1306 controller." VALUE_FUN atmega_regmap VALUE_LIST atmega_ddr_values CONFIG_FILE "oled_avr_config")
setup_option_def(OLED_AVR_PORT_SCL PORTB INT "SCL PORT register for I2C of SSD1306 controller." VALUE_FUN atmega_regmap VALUE_LIST atmega_port_values CONFIG_FILE "oled_avr_config")
setup_option_def(OLED_AVR_PORT_SDA PORTB INT "SDA PORT register for I2C of SSD1306 controller." VALUE_FUN atmega_regmap VALUE_LIST atmega_port_values CONFIG_FILE "oled_avr_config")
setup_option_def(OLED_AVR_PIN_NUM_SCL 0 INT "SCL pin number for I2C of SSD1306 controller." VALUE_LIST atmega_pinnum_values CONFIG_FILE "oled_avr_config")
setup_option_def(OLED_AVR_PIN_NUM_SDA 1 INT "SDA pin number for I2C of SSD1306 controller." VALUE_LIST atmega_pinnum_values CONFIG_FILE "oled_avr_config")