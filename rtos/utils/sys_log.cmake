# Setup configuration options.
setup_option_def(SYS_LOG_NONE ON DEFINE "Disables system log for all the components." CONFIG_FILE "sys_log_config")
setup_option_def(SYS_LOG_RUNTIME_UPDATE OFF DEFINE "Enable run time updates of system log levels of all components." CONFIG_FILE "sys_log_config")

setup_option_def(SYS_LOG_DEFAULT "SYS_LOG_INFO|SYS_LOG_WARN|SYS_LOG_ERROR" MACRO "Setup default system log level for all components." CONFIG_FILE "sys_log_config")
set(sys_log_levels SYS_LOG_FUNCTION_CALL SYS_LOG_DEBUG SYS_LOG_INFO SYS_LOG_WARN SYS_LOG_ERROR SYS_LOG_ALL)
setup_option_list_values(SYS_LOG_DEFAULT sys_log_levels "|")