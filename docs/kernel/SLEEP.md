Sleep
=====
## Introduction
Sleep is used to control timed operations in an operating system. This not only allows a task to suspend for a specific amount of time but also allows condition to implement timeouts and timer when required by an application.

### Sources
- [sleep.c](../../rtos/kernel/sleep.c)

### Headers
- [sleep.h](../../rtos/kernel/sleep.h)

## Basic Concepts
On each tick sleep is invoked to see if there is a thread that can be resumed. If a thread is found it is moved to the ready task list and is scheduled if required. In case the scheduler is locked by the running task. The sleep is invoked when the running task unlocks the scheduler.

## APIs
### sleep\_ticks
This API will suspend the current task for the number of provided software ticks. As system clock rate can change so it is not recommended to call this API directly, use [sleep_ms](SLEEP.md#sleep_ms) instead.  
**takes** the number of ticks to suspend the current task.  
Implemented by [sleep.c](../../rtos/kernel/sleep.c).

### sleep\_hw\_ticks
This API will block the current task for the number of provided hardware ticks. This API performs a busy wait and is not recommended. As system clock rate can change so it is not recommended to call this API directly, use [sleep_us](SLEEP.md#sleep_us) instead.  
**takes** the number of hardware ticks to block the current task.  
Implemented by [sleep.c](../../rtos/kernel/sleep.c).

## Helper Macros
### sleep\_ms
This suspends the current task for the given number of milliseconds. If the provided milliseconds are not granule with the system tick, they will be wrapped to the smallest tick after which we need to resume this task.  
**takes** the number of milliseconds to suspend the current task.  
Implemented by [sleep.h](../../rtos/kernel/sleep.h).

### sleep\_us
This suspends the current task for the given number of microseconds. This APIs is not recommended to use as this performs a busy wait.  
**takes** the number of microseconds to block the current task.  
Implemented by [sleep.h](../../rtos/kernel/sleep.h).