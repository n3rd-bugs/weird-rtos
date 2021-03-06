# Setup configuration options.
setup_option_def(SERIAL_BAUD_RATE 115200 INT "USART boudrate." CONFIG_FILE "usart_stm32_config")
setup_option_def(SERIAL_INTERRUPT_MODE OFF DEFINE "Use interrupts to transfer data over USART." CONFIG_FILE "usart_stm32_config")
setup_option_def(SERIAL_DEBUG_CONSOLE ON BOOL "Use serial for debug console." CONFIG_FILE "usart_stm32_config")
if (${SERIAL_INTERRUPT_MODE})
    setup_option_def(SERIAL_MAX_BUFFER_SIZE 16 INT "Buffer size for the serial device (interrupt mode)." CONFIG_FILE "usart_stm32_config")
    setup_option_def(SERIAL_NUM_BUFFERS 4 INT "Number of buffer for the serial device (interrupt mode)." CONFIG_FILE "usart_stm32_config")
    setup_option_def(SERIAL_NUM_BUFFER_LIST 4 INT "Number of buffer lists for the serial device (interrupt mode)." CONFIG_FILE "usart_stm32_config")
    setup_option_def(SERIAL_THRESHOLD_BUFFER 0 INT "Number of threshold buffers for the serial device (interrupt mode)." CONFIG_FILE "usart_stm32_config")
    setup_option_def(SERIAL_THRESHOLD_BUFFER_LIST 0 INT "Number of threshold buffer lists for the serial device (interrupt mode)." CONFIG_FILE "usart_stm32_config")
endif ()