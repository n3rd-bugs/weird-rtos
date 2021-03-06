# Include helpers.
include(${RTOS_ROOT}/cmake/modules/helper.cmake)

# Initialize RTOS configurations.
setup_option(CONFIG_FS ON)
setup_option(CONFIG_NET ON)
setup_option(IO_I2C ON)
setup_option(I2C_PCF8574 ON)
setup_option(IO_SPI ON)
setup_option(IO_MMC OFF)
setup_option(IO_ADC ON)
setup_option(IO_LCD_AN ON)
setup_option(LCD_PCF8574 ON)
setup_option(IO_ETHERNET ON)
setup_option(CONFIG_WEIRD_VIEW ON)

# Setup IDLE task options.
setup_option(IDLE_WORK_MAX 1)
setup_option(IDLE_TASK_STACK_SIZE 192)

# Update the number of ticks per second to 10.
setup_option(SOFT_TICKS_PER_SEC 10)

# Setup task options.
setup_option(TASK_STATS ON)
setup_option(TASK_USAGE ON)
setup_option(TASK_USAGE_RETAIN ON)

# Setup enc28j60 configurations.
setup_option(ENC28J60_INT_POLL ON)
setup_option(ENC28J60_MAX_BUFFER_SIZE 64)
setup_option(ENC28J60_NUM_BUFFERS 32)
setup_option(ENC28J60_NUM_BUFFER_LISTS 8)
setup_option(ENC28J60_NUM_THR_BUFFER 2)
setup_option(ENC28J60_NUM_THR_LIST 2)
setup_option(ENC28J60_NUM_ARP 2)
setup_option(ENC28J60_NUM_IPV4_FRAGS 0)
setup_option(ENC28J60_SOFT_MAX_RX_FRAME 2048)

# Setup networking stack configurations.
setup_option(NET_COND_STACK_SIZE 512)
setup_option(IPV4_ENABLE_FRAG OFF)
setup_option(IPV4_ALLOW_SIZE_MISMATCH ON)
setup_option(UDP_ALLOW_SIZE_MISMATCH ON)
setup_option(UDP_CSUM OFF)
setup_option(NET_NUM_ROUTES 4)
setup_option(NET_TCP OFF)

# Setup static IP configuration.
# setup_option(NET_DHCP OFF)
# setup_option(ENC28J60_DEFAULT_IP 0xC0A80103)

# Setup AVR configurations.
setup_option(ADC_AVR_PRESCALE ADC_AVR_DIV_64)
setup_option(LCD_AN_AVR_PIN_SCL PIND)
setup_option(LCD_AN_AVR_PIN_SDA PIND)
setup_option(LCD_AN_AVR_DDR_SCL DDRD)
setup_option(LCD_AN_AVR_DDR_SDA DDRD)
setup_option(LCD_AN_AVR_PORT_SCL PORTD)
setup_option(LCD_AN_AVR_PORT_SDA PORTD)
setup_option(LCD_AN_AVR_PIN_NUM_SCL 0)
setup_option(LCD_AN_AVR_PIN_NUM_SDA 1)

# Setup target configuration.
setup_option(TGT_PLATFORM atmega1284p)
#setup_option(TGT_PLATFORM atmega644p)
setup_option(F_CPU 20000000UL)

# Setup bootloader
setup_option(CONFIG_BOOTLOAD ON)
setup_option(BOOTLOAD_MMC OFF)