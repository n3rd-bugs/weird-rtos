# Setup configuration options.
setup_option_def(LCD_AN_DEBUG ON DEFINE "Enable debug information to be printed over registered LCD." CONFIG_FILE "lcd_an_config")
setup_option_def(LCD_AN_NO_BUSY_WAIT OFF DEFINE "Enable busy wait for LCD controller." CONFIG_FILE "lcd_an_config")
setup_option_def(LCD_AN_BUSY_TIMEOUT 100 INT "Maximum number of milliseconds to wait for LCD busy bit to be toggled." CONFIG_FILE "lcd_an_config")
setup_option_def(LCD_AN_TAB_SIZE 3 INT "Tab size." CONFIG_FILE "lcd_an_config")
setup_option_def(LCD_AN_INIT_DELAY 0 INT "Number of milliseconds to wait before initializing LCD." CONFIG_FILE "lcd_an_config")
setup_option_def(LCD_AN_CLEAR_DELAY 10 INT "Number of milliseconds to wait after a clear command." CONFIG_FILE "lcd_an_config")
setup_option_def(LCD_AN_READ_DELAY 5 INT "Number of milliseconds to wait before reading a register from LCD." CONFIG_FILE "lcd_an_config")