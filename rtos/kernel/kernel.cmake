# Setup kernel configuration options.
if (${CONFIG_SLEEP})
setup_option_def(SOFT_TICKS_PER_SEC 100 UINT32 "Configure the number of ticks per second." CONFIG_FILE "kernel_config")
endif ()
