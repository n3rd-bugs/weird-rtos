# Setup configuration options.
setup_option_def(TCP_WND_SIZE 1024 INT "TCP window size in bytes." CONFIG_FILE "net_tcp_config")
setup_option_def(TCP_WND_SCALE 2 INT "TCP window scale to be used." CONFIG_FILE "net_tcp_config")
setup_option_def(TCP_RTO 750 INT "TCP base retransmission timeout." CONFIG_FILE "net_tcp_config")
setup_option_def(TCP_MAX_RTO 5000 INT "TCP maximum retransmission timeout." CONFIG_FILE "net_tcp_config")
setup_option_def(TCP_MSL 60000 INT "TCP maximum segment length." CONFIG_FILE "net_tcp_config")
setup_option_def(TCP_NUM_RTX 16 INT "TCP maximum retransmissions." CONFIG_FILE "net_tcp_config")
setup_option_def(TCP_MAX_CONG_WINDOW 0xFFFF INT "TCP congestion window size." CONFIG_FILE "net_tcp_config")