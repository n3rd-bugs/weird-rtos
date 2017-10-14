# Setup configuration options.
setup_option_def(LCD_AN_STM32_ROWS 4 INT "Number of rows in the alphanumeric LCD.")
setup_option_def(LCD_AN_STM32_COLS 20 INT "Number of columns in the alphanumeric LCD.")

# Valid pin numbers for PCF8574.
set(pcf8574_pinnum_values 0 1 2 3 4 5 6 7 CACHE INTERNAL "" FORCE)

if (${CONFIG_LCD_PCF8574})
    setup_option_def(LCD_AN_STM32_I2C_ADDRESS 0x3F INT "I2C address for LCD GPIO controller.")
    setup_option_def(LCD_AN_STM32_PIN_RW 1 INT "Read write (RW) pin for alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values)
    setup_option_def(LCD_AN_STM32_PIN_RS 0 INT "Register select (RS) pin for alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values)
    setup_option_def(LCD_AN_STM32_PIN_EN 2 INT "Enable (EN) pin for alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values)
    setup_option_def(LCD_AN_STM32_PIN_D4 4 INT "Data 4 (D4) pin for alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values)
    setup_option_def(LCD_AN_STM32_PIN_D5 5 INT "Data 5 (D5) pin for alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values)
    setup_option_def(LCD_AN_STM32_PIN_D6 6 INT "Data 6 (D6)s pin for alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values)
    setup_option_def(LCD_AN_STM32_PIN_D7 7 INT "Data 7 (D7) pin for alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values)
    setup_option_def(LCD_AN_STM32_PIN_BL 3 INT "Back light pin for alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values)
endif ()