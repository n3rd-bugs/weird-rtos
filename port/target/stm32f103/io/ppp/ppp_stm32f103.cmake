# Setup configuration options.
setup_option_def(PPP_BAUD_RATE 115200 INT "PPP over USART boudrate." CONFIG_FILE "ppp_stm32_config")
setup_option_def(PPP_MAX_BUFFER_SIZE 64 INT "Buffer size for the PPP over serial device (interrupt mode)." CONFIG_FILE "ppp_stm32_config")
setup_option_def(PPP_NUM_BUFFERS 16 INT "Number of buffer for the PPP over serial device (interrupt mode)." CONFIG_FILE "ppp_stm32_config")
setup_option_def(PPP_NUM_BUFFER_LIST 8 INT "Number of buffer lists for the PPP over serial device (interrupt mode)." CONFIG_FILE "ppp_stm32_config")
setup_option_def(PPP_THRESHOLD_BUFFER 2 INT "Number of threshold buffers for the PPP over serial device (interrupt mode)." CONFIG_FILE "ppp_stm32_config")
setup_option_def(PPP_THRESHOLD_BUFFER_LIST 2 INT "Number of threshold buffer lists for the PPP over serial device (interrupt mode)." CONFIG_FILE "ppp_stm32_config")