# Setup configuration options.
setup_option_def(UDP_CSUM ON DEFINE "Enables checksum calculation and validation for UDP datagrams." CONFIG_FILE "net_udp_config")
setup_option_def(UDP_ALLOW_SIZE_MISMATCH OFF DEFINE "Allow frames to be passed to upper layers if the length does not match the header." CONFIG_FILE "net_udp_config")