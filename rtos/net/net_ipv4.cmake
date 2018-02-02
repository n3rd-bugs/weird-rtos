# Setup configuration options.
setup_option_def(IPV4_ENABLE_FRAG ON DEFINE "Enable IPv4 fragmentation." CONFIG_FILE "net_ipv4_config")
setup_option_def(IPV4_FRAG_TIMEOUT 60000 INT "Time in milliseconds after which a IPv4 fragment is dropped." CONFIG_FILE "net_ipv4_config")
setup_option_def(IPV4_FRAG_DROP_TIMEOUT 5000 INT "Time in milliseconds after which a IPv4 fragment list is dropped if we ran out of buffers." CONFIG_FILE "net_ipv4_config")