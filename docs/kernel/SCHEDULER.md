Scheduler
=========
## Introduction
Weird RTOS provides a basic priority based aperiodic scheduler. This is implemented by [scheduler.c](weird-rtos/blob/master/weird-rtos/blob/master/rtos/kernel/scheduler.c) and [kernel.c](weird-rtos/blob/master/weird-rtos/blob/master/rtos/kernel/kernel.c).

## Basic Concepts
### Scheduling
A task is scheduled if it is the highest priority task on the ready list. If there are more than one ready tasks with same priority, control will be provided to the oldest task. A running task can only be suspended by *sleep* or a *condition* and can only be scheduled out by an *interrupt*.

### System tick
Kernel requires a dedicated timer to generate a system tick at which scheduler is invoked to see if we require a context switch. This also helps in tracking the system clock and driving any timed conditions.

### Task switching during interrupt
A task can become ready if an interrupt resumes it. In such case running task will only be scheduled out if the new ready task has higher or equal priority.

## Configurations
### SCHEDULER\_MAX\_LOCK
Configures the maximum number of nested scheduler locks that can be obtained, this will raise an assert if assert is enabled. This option is helpful in debugging scheduler locks.

## APIs
### scheduler\_init
This API must be called before invoking scheduler. This initializes scheduler internals.
Implemented by [scheduler.c](weird-rtos/blob/master/rtos/kernel/scheduler.c).

### scheduler\_lock
This API will lock the context of current task (interrupts can still schedule out this task), If an other higher priority task becomes ready, context switch will remain pended until scheduler is unlocked.
Implemented by [scheduler.c](weird-rtos/blob/master/rtos/kernel/scheduler.c).

### scheduler\_unlock
This API will unlock the context of current task. This will invoke the scheduler if the system has missed a scheduling point. The context may be scheduled out if a higher priority task is now ready.
Implemented by [scheduler.c](weird-rtos/blob/master/rtos/kernel/scheduler.c).

### task\_yield
This API will try to yield current task. Task will only be switched if an higher or same priority task is found on the ready list.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).

### get\_current\_task
This API will return the control block of the the current task. Null will be returned if called from interrupt service routine.
**returns** the control block of current task.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).

### current\_tick
This API will return the system clock's current tick. Application can use helper macros to convert this tick into actual time.
**returns** the current system tick.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).

### kernel\_run
This API will start the system scheduler, this should be called once the system is initialized.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).

## Helper Macros
### SOFT\_TICKS_PER_SEC
This is the number of clock ticks per second.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).

### MS\_TO\_TICK
This converts the given milliseconds to the number of equivalent system ticks.
**takes** number of milliseconds.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).

### TICK\_TO\_MS
This converts the given ticks to the number of equivalent milliseconds.
**takes** number of ticks.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).

### US\_TO\_HW\_TICK
This converts the given milliseconds to the number of equivalent hardware ticks.
**takes** number of milliseconds.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).

### HW\_TICK\_TO\_US
This converts the given hardware ticks to the number of equivalent milliseconds.
**takes** number of hardware ticks.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).

### ISR\_ENTER
This must be the first statement of an interrupt service routine.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).

### ISR\_EXIT
This must be the last statement of an interrupt service routine.
Implemented by [kernel.c](weird-rtos/blob/master/rtos/kernel/kernel.c).
