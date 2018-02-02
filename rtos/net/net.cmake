# Setup configuration options.
setup_option_def(NET_IPV4 ON DEFINE "Enables IPv4 stack." CONFIG_FILE "net_config")
setup_option_def(NET_ICMP ON DEFINE "Enables ICMP over IPv4 protocol." CONFIG_FILE "net_config")
setup_option_def(NET_UDP ON DEFINE "Enables UDP over IPv4 protocol." CONFIG_FILE "net_config")
setup_option_def(NET_TCP ON DEFINE "Enables TCP over IPv4 protocol." CONFIG_FILE "net_config")
setup_option_def(NET_ARP ON DEFINE "Enables ARP protocol for IPv4 address resolution." CONFIG_FILE "net_config")
setup_option_def(NET_DHCP ON DEFINE "Enables IPv4 DHCP protocol." CONFIG_FILE "net_config")