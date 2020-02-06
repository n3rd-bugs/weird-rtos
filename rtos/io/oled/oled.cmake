# Setup configuration options.
setup_option_def(OLED_I2C_CHUNK_SIZE 16 INT "Size of I2C message size." CONFIG_FILE "oled_config")
setup_option_def(OLED_SSD1306_INIT_DELAY 500 INT "ms to wait for the SSD1306 to initialize" CONFIG_FILE "oled_config")