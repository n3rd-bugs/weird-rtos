Purpose of this page is to provide any useful information about using weird-rtos
on ATMEGA644P platform.

###Eclipse configuration:
    1. You must have installed CDT plug-in and a GNU-AVR toolchain on your
       host.
    2. Create a new empty AVR cross target application project.
    3. Copy content of the GIT repository in the empty project.
    4. Open the properties of this project.
        a. In "C/C++ Build" go to settings, in "Tools Settings"
            AVR Compiler->Directories->Include paths (-I): copy the 
                "C Includes" from the "eclipse_config.txt".
    5. Now delete or exclude from build unrelated files:
        a. All other architectures from "port/os" and keep "avr".
        b. All other targets from "port/target" and keep "atmega644p".
        c. All other tool-chains from "port/toolset" and keep "gcc-avr".
    6. Open os_target.h and provide configuration:
        #define PLAT_TARGET         PLAT_ATMEGA644P
        #define RTOS_TARGET         TARGET_AVR
        #define TOOL_TARGET         TOOL_AVR_GCC
    7. Select an appropriate sample and exclude all other samples from the build.
        a. For "enc28j60_demo.c" connect the enc28j60 as provided configuration
            PD2 -> INT
            PD4 -> POW-RST
            PB4 -> CS
            PB7 -> SCLK
            PB6 -> SO
            PB5 -> SI
           Open "config.h" and provide this configuration
            #define CONFIG_SLEEP
            #define CONFIG_FS
            #define CONFIG_NET
            #define CONFIG_SPI
            #define CONFIG_ETHERNET
           Open "enc28j60.h" and update following configurations
            #define ENC28J60_MAX_BUFFER_SIZE    (32)
            #define ENC28J60_NUM_BUFFERS        (24)
            #define ENC28J60_NUM_BUFFER_LISTS   (12)
            #define ENC28J60_NUM_THR_BUFFER     (8)
           Open "net_condition.h" and update following configuration
            #define NET_COND_STACK_SIZE     512
           Build and execute the project.
           On Host build the "udp_echo_host.c" and execute after modifying the 
           target IP address that can obtained from the DHCP server running on 
           the network and updating the PACKET_SIZE to (64).