Scheduler
=========
## Introduction
Weird RTOS provides a basic priority based aperiodic scheduler. This is implemented by [scheduler.c](rtos/kernel/scheduler.c) and user APIs are provided by [kernel.c](rtos/kernel/kernel.c).

## Basic Concepts
### Scheduling
A task is scheduled if it is the highest priority task on the ready list. If there are more than one ready tasks with same priority, control will be provided to the oldest task. A running task can only be suspended by *sleep* or a *condition* and can only be scheduled out by an *interrupt*.

### Task switching during interrupt
A task can become ready if an interrupt resumes it. In such case running task will only be scheduled out if the new ready task has higher or equal priority.

## APIs
### scheduler_init
This API must be called before invoking scheduler. This initializes scheduler internals.

### scheduler_lock
This API will lock the context of current task (interrupts can still schedule out this task), If an other higher priority task becomes ready, context switch will remain pended until scheduler is unlocked.

### scheduler_unlock
This API will unlock the context of current task. This will invoke the scheduler if the system has missed a scheduling point. The context may be scheduled out if a higher priority task is now ready.