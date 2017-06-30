LOGGING
=======
## Introduction
Weird RTOS provides whole sum solution to trace system information using debug console using its logging infrastructure that includes assert, system log and system information APIs.

### Sources
- [assert.c](../../rtos/kernel/assert.c)
- [sys_info.c](../../rtos/kernel/sys_info.c)
- [sys_log.c](../../rtos/utils/sys_log.c)

### Headers
- [assert.h](../../rtos/kernel/assert.h)
- [sys_info.h](../../rtos/kernel/sys_info.h)
- [sys_log.h](../../rtos/utils/sys_log.h)

## Basic Concepts
During development sometimes it is not possible to track bugs through debug connection. To handle such situation debug console is used to print out useful information at the time of crash. Weird RTOS provides configurable scalable logging facilities that not only allows tracing issues in RTOS but also in user application.

- *Assert*
Assert provides APIs to detect and halt system at a certain non-recoverable system crash. This also allows a user to display the origin of such crash. However it is application's responsibility to implement any recovery from such situation if possible.

- *System Information*
This module provides APIs to compute and display system information such as stack usage of various tasks in the system. This allows user to track stack overflow in system and even optimize stack for a certain application.

- *System Log*
System log provides runtime tracing facility with component level log filtering support. This allows a user to configure logs from a specific component and track root cause of pesky issues. However due to the nature of this service it is not advisable to be used in the core components as it will not only hide the actual issue but also may cause undefined behaviour.

## Configurations
### ASSERT\_NONE
If defined will disable all the assert condition handling. This should be defined in production.

### ASSERT\_FILE\_INFO
If defined the assert routine will include the file/line information in the assert log.

### SYS\_LOG\_NONE
If defined will disable system log. 

### SYS\_LOG\_RUNTIME\_UPDATE
If defined allows application to dynamically adjust the various log levels.

### SYS\_LOG\_DEFAULT
This defines the default system log level.

### SYS\_LOG\_LEVEL\_*XXX*
These allow user to configure log level of various components in the system.

## APIs
### system\_assert
This function will halt the system and print the file/line and task information if required. In most cases direct call to this API can be avoided by using [ASSERT](LOGGING.md#ASSERT) and [ASSERT\_INFO](LOGGING.md#ASSERT_INFO). A user application can update this API to implement its own assert handler.
**takes** optional argument that can be used to identify the origin of an exception.
**takes** file name from which assert condition was detected.
**takes** line number at which this assert condition was detected.
**takes** control block of the current task.
Implemented by [assert.c](../../rtos/kernel/assert.c).

### util\_task\_calc\_free\_stack
This function calculate and returns number of bytes free on a given task's stack.
**takes** control block of the task for which we need to calculate stack usage.
Implemented by [sys_info.c](../../rtos/kernel/sys_info.c).

### util\_system\_calc\_free\_stack
This function returns the number of bytes free on the system stack.
Implemented by [sys_info.c](../../rtos/kernel/sys_info.c).

### util\_print\_sys\_info
This function prints generalized information about the operating system. This includes system up time and stack usage of various tasks.
Implemented by [sys_info.c](../../rtos/kernel/sys_info.c).

### util\_print\_sys\_info\_assert
This function prints generalized information about the operating system on the serial port in assert mode.
Implemented by [sys_info.c](../../rtos/kernel/sys_info.c).

### util\_print\_sys\_info\_buffer
This function prints generalized information about the operating system in the given file system buffer.
Implemented by [sys_info.c](../../rtos/kernel/sys_info.c).

### sys\_log
This function logs a debug message on the console.
**takes** the component name from which this message was printed.
**takes** a formated message to be logged.
**takes** various formating arguments.
Implemented by [sys_log.c](../../rtos/utils/sys_log.c).

### sys\_log\_hexdump
This function logs a debug message on the console along with HEX dump of provided memory region.
**takes** the component name from which this message was printed.
**takes** a formated message to be logged.
**takes** memory region to be dumped in HEX.
**takes** number of bytes in memory region to be dumped.
**takes** various formating arguments.
Implemented by [sys_log.c](../../rtos/utils/sys_log.c).

## Helper Macros
### ASSERT
This will assert the system if the given argument is not false. If assert is triggered it will be provided the file and line information so that the origin of assert can be detected.
**takes** an argument that if is not false, assert will be triggered.
Implemented by [assert.h](../../rtos/kernel/assert.h).

### ASSERT\_INFO
This is same as [ASSERT](LOGGING.md#ASSERT) but user also provide the file and line information to be passed to assert. This can be used to track the call hierarchy of exception.
**takes** an argument that if is not false, assert will be triggered.
**takes** file name to be passed to assert.
**takes** line number to be passed to assert.
Implemented by [assert.h](../../rtos/kernel/assert.h).

### SYS\_LOG\_MSG
This logs the message for the provided component and logging level.
**takes** component identifier for which we are logging this message.
**takes** log level of this message.
**takes** a formated message.
**takes** various format arguments.
Implemented by [sys_log.h](../../rtos/utils/sys_log.h).

### SYS\_LOG\_FUNCTION\_MSG
This is like [ASSERT](LOGGING.md#SYS_LOG_MSG) but also logs the function name from which this message was logged.
**takes** component identifier for which we are logging this message.
**takes** log level of this message.
**takes** a formated message.
**takes** various format arguments.
Implemented by [sys_log.h](../../rtos/utils/sys_log.h).

### SYS\_LOG\_FUNCTION\_ENTRY
This logs the entry of a function.
**takes** component identifier for which we are logging this message.
Implemented by [sys_log.h](../../rtos/utils/sys_log.h).

### SYS\_LOG\_FUNCTION\_EXIT\_STATUS
This logs the exit of a function with the given status. If status is not success the log level is increased to error.
**takes** component identifier for which we are logging this message.
**takes** the return status of function.
Implemented by [sys_log.h](../../rtos/utils/sys_log.h).

### SYS\_LOG\_FUNCTION\_EXIT
This logs the exit of a function.
**takes** component identifier for which we are logging this message.
Implemented by [sys_log.h](../../rtos/utils/sys_log.h).

### SYS\_LOG\_HEXDUMP
This logs a message and also includes a HEX dump of given memory region.
**takes** component identifier for which we are logging this message.
**takes** log level of this message.
**takes** a formated message.
**takes** memory region to be dumped in HEX.
**takes** number of bytes in memory region to be dumped.
**takes** various format arguments.
Implemented by [sys_log.h](../../rtos/utils/sys_log.h).

### SYS\_LOG\_FUNCTION\_HEXDUMP
This is like [ASSERT](LOGGING.md#SYS_LOG_HEXDUMP) but also logs the function name from which this message was logged.
**takes** component identifier for which we are logging this message.
**takes** log level of this message.
**takes** a formated message.
**takes** memory region to be dumped in HEX.
**takes** number of bytes in memory region to be dumped.
**takes** various format arguments.
Implemented by [sys_log.h](../../rtos/utils/sys_log.h).