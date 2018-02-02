# Setup IDLE task configuration options.
setup_option_def(IDLE_RUNTIME_UPDATE OFF DEFINE "Configure runtime IDLE work updates." CONFIG_FILE "idle_config")
setup_option_def(IDLE_WORK_MAX 0 INT "Maximum number of IDLE works in the system." CONFIG_FILE "idle_config")
setup_option_def(IDLE_TASK_STACK_SIZE 64 INT "IDLE task stack size." CONFIG_FILE "idle_config")
