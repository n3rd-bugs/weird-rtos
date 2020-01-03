# Setup configuration options.
set(max7219_intensity_values 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 CACHE INTERNAL "" FORCE)
setup_option_def(MAX7219_INTENSITY 0 INT "Configure intensity for MAX7219" VALUE_LIST max7219_intensity_values  CONFIG_FILE "max7219_config")