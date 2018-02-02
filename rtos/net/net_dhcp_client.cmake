# Setup configuration options.
setup_option_def(DHCP_BASE_TIMEOUT 2000 INT "DHCP client backoff time base in milliseconds." CONFIG_FILE "net_dhcp_client_config")
setup_option_def(DHCP_MAX_TIMEOUT 64000 INT "DHCP client maximum backoff time in milliseconds." CONFIG_FILE "net_dhcp_client_config")
setup_option_def(DHCP_MAX_RETRY 4 INT "Maximum number of DHCP retries before client moves back to discovery state." CONFIG_FILE "net_dhcp_client_config")
setup_option_def(DHCP_CLIENT_HOSTNAME "weird-rtos" STRING "DHCP client hostname." CONFIG_FILE "net_dhcp_client_config")