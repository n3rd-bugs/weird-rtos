# Setup configuration options.
setup_option_def(LCD_AN_STM32_ROWS 4 INT "Number of rows in the Alphanumeric LCD." CONFIG_FILE "lcd_an_stm32_config")
setup_option_def(LCD_AN_STM32_COLS 20 INT "Number of columns in the Alphanumeric LCD." CONFIG_FILE "lcd_an_stm32_config")

# Valid pin numbers for PCF8574.
set(pcf8574_pinnum_values 0 1 2 3 4 5 6 7 CACHE INTERNAL "" FORCE)

if (${LCD_PCF8574})
    setup_option_def(LCD_AN_STM32_I2C_ADDRESS 0x3F INT "I2C address for Alphanumeric LCD GPIO controller." CONFIG_FILE "lcd_an_stm32_config")
    setup_option_def(LCD_AN_STM32_PIN_RW 1 INT "Read write (RW) pin for Alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values CONFIG_FILE "lcd_an_stm32_config")
    setup_option_def(LCD_AN_STM32_PIN_RS 0 INT "Register select (RS) pin for Alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values CONFIG_FILE "lcd_an_stm32_config")
    setup_option_def(LCD_AN_STM32_PIN_EN 2 INT "Enable (EN) pin for Alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values CONFIG_FILE "lcd_an_stm32_config")
    setup_option_def(LCD_AN_STM32_PIN_D4 4 INT "Data 4 (D4) pin for Alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values CONFIG_FILE "lcd_an_stm32_config")
    setup_option_def(LCD_AN_STM32_PIN_D5 5 INT "Data 5 (D5) pin for Alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values CONFIG_FILE "lcd_an_stm32_config")
    setup_option_def(LCD_AN_STM32_PIN_D6 6 INT "Data 6 (D6)s pin for Alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values CONFIG_FILE "lcd_an_stm32_config")
    setup_option_def(LCD_AN_STM32_PIN_D7 7 INT "Data 7 (D7) pin for Alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values CONFIG_FILE "lcd_an_stm32_config")
    setup_option_def(LCD_AN_STM32_PIN_BL 3 INT "Back light pin for Alphanumeric LCD over I2C." VALUE_LIST pcf8574_pinnum_values CONFIG_FILE "lcd_an_stm32_config")
endif ()