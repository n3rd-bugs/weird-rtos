# Setup configuration options.
setup_option_def(ENC28J60_ATMEGA644P_RESET_DELAY 100 MACRO "Time in miliseconds to assert the reset line of ENC28J60 ethernet controller.")
setup_option_def(ENC28J60_USE_SPI_BB OFF BOOL "Use bit band SPI for ENC28J60 ethernet controller. Needs manual modifications to configure bit bang interface if default SPI is not used.")