# Setup configuration options.
setup_option_def(NET_COND_NUM_DEVICES 2 INT "Number of devices that will be registered with networking condition task." CONFIG_FILE "net_condition_config")
setup_option_def(NET_COND_NUM_INTERNAL 7 INT "Number of conditions to be reserved for internal operations." CONFIG_FILE "net_condition_config")
setup_option_def(NET_COND_STACK_SIZE 1024 INT "Stack size for networking condition task." CONFIG_FILE "net_condition_config")