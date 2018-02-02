# Setup tasks configuration options.
setup_option_def(TASK_STATS OFF DEFINE "Enable tasks statistics including task stack usage." CONFIG_FILE "tasks_config")
setup_option_def(TASK_USAGE OFF DEFINE "Enable CPU usage statistics for tasks." CONFIG_FILE "tasks_config")
setup_option_def(TASK_STACK_PATTERN A CHAR "Pattern character to be used to fill a task stack." CONFIG_FILE "tasks_config")
