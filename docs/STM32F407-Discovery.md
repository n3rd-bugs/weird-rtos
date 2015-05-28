Purpose of this page is to provide any useful information about using weird-rtos
on STM32F407-Discovery platform.

###Eclipse configuration:
    1. You must have installed CDT plug-in and a GNU-GCC tool chain on your host.
    2. Create a new empty executable c project with required cross-toolchain.
    3. Copy content of the GIT repository in the empty project.
    4. Open the properties of this project.
        a. In "C/C++ Build" go to settings, in "Tools Settings"
            Target Processor: select "Cortex-M4", "FP Instruction (hard)", 
                "fpv4-sp-d16".
            Cross ARM Compiler->Includes->Include paths (-I): copy the 
                "C Includes" from the "eclipse_config.txt".
            Cross ARM Linker->General->Script Files (-T): add the "STM32F407.ld"
            Cross ARM Linker->Miscellaneous->Other linker flags: set 
                --specs=nosys.specs
    5. Now delete or exclude from build unrelated files:
        a. All other architectures from "port/os" and keep "cortex-m4".
        b. All other targets from "port/target" and keep "stm32f407discovery".
        c. All other tool-chains from "port/toolset" and keep "gcc-arm".
        d: Also exclude any files/folders mentioned in "Exclude" in 
           "eclipse_config.txt"
    6. Open os_target.h and provide configuration:
        #define PLAT_TARGET         PLAT_STM32F407_DISC
        #define RTOS_TARGET         TARGET_CORTEX_M4
        #define TOOL_TARGET         TOOL_ARM_GCC
    7. Select an appropriate sample and exclude all other samples from the build.
        a. For "enc28j60_demo.c" connect the device as provided configuration
            PA2 -> INT
            PA3 -> POW-RST
            PA4 -> CS
            PA5 -> SCLK
            PA6 -> SO
            PA7 -> SI
           Open "config.h" and provide this configuration
            #define CONFIG_SLEEP
            #define CONFIG_MEMGR
            #define CONFIG_FS
            #define CONFIG_NET
            #define CONFIG_SPI
            #define CONFIG_ETHERNET
           Build and execute the project.
           On Host build the "udp_echo_host.c" and execute after modifying the 
           target IP address that can obtained from the DHCP server running on 
           the system. We can get around 300KB/s speed for echo test. If ran 
           with only TX from target side the speeds are up-to 540KB/s. These 
           improve  very little if baud-rate is increased to 42MHz, but every 
           thing remains stable.