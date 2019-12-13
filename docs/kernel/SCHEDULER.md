Scheduler
=========
## Introduction
Weird RTOS provides a basic priority based aperiodic scheduler.

### Sources
- [scheduler.c](../../rtos/kernel/scheduler.c)
- [tasks.c](../../rtos/kernel/tasks.c).
- [kernel.c](../../rtos/kernel/kernel.c).

### Headers
- [scheduler.h](../../rtos/kernel/scheduler.h)
- [tasks.h](../../rtos/kernel/tasks.h)
- [kernel.h](../../rtos/kernel/kernel.h)

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

### CONFIG\_TASK\_STATS
Configures if we need to collect task statistics.

### CONFIG\_TASK\_USAGE
Configures if we need to collect task CPU usage statistics.

### CONFIG\_STACK\_PATTERN
Configures the pattern to be filled in the task's stack to track stack usage.

## Data Structures
### TASK\_ENTRY
This defines a task's entry function.

```
typedef void TASK_ENTRY (void *argv);
```

### TASK
This defines control block for a task.

```
typedef struct _task
{
#if (defined(TASK_STATS) && defined(TASK_USAGE))
    /* Number of ticks this task was scheduled. */
    uint64_t    total_active_ticks;

    /* Tick at which this task was last scheduled. */
    uint64_t    last_active_tick;
#endif /* (defined(TASK_STATS) && defined(TASK_USAGE)) */

    /* Task entry function. */
    TASK_ENTRY  *entry;

    /* Task arguments. */
    void        *argv;

    /* Task list member. */
    TASK        *next;

#ifdef TASK_STATS
    /* Number of times this task was scheduled. */
    uint32_t    scheduled;

    /* Global task list member. */
    TASK        *next_global;

    /* This is start of the stack pointer for this task. */
    uint8_t     *stack_start;

    /* Task stack size. */
    uint32_t    stack_size;

    /* Name for this task. */
    char        name[8];
#endif /* TASK_STATS */

    /* This holds current stack pointer of this task. */
    uint8_t     *tos;

    /* If suspended this will hold task suspension data. */
    void        *suspend_data;

#ifdef CONFIG_SLEEP
    /* Link list member for sleeping tasks. */
    TASK        *next_sleep;

    /* The system tick at which this task is needed to be rescheduled. */
    uint32_t    tick_sleep;
#endif /* CONFIG_SLEEP */

    /* Current task status. */
    int32_t     status;

#if (defined(TASK_STATS) && defined(TASK_USAGE))
    /* Structure padding. */
    int8_t      pad[4];
#endif

    /* Number of wait conditions on which this task is waiting. */
    uint8_t     num_conditions;

    /* This defines task priority. */
    uint8_t     priority;

    /* Task flags as configured by scheduler. */
    uint8_t     flags;

    /* Lock count, how much nested scheduler locks have we acquired. */
    uint8_t     lock_count;
} TASK;
```

## APIs
### scheduler\_init
This API must be called before invoking scheduler. This initializes scheduler internals.
Implemented by [scheduler.c](../../rtos/kernel/scheduler.c).

### scheduler\_task\_add
This API adds a task in the scheduler. [task_create](SCHEDULER.md#task_create) must be called to initilize a task's control block before adding it in the scheduler.
**takes** the task control block that is needed to be added in the scheduler.
**takes** the priority for this task.
Implemented by [scheduler.c](../../rtos/kernel/scheduler.c).

### scheduler\_task\_remove
Removes a finished task from the scheduler.
**takes** the task control block that is needed to be removed from scheduler.
Implemented by [scheduler.c](../../rtos/kernel/scheduler.c).

### scheduler\_lock
This API will lock the context of current task (interrupts can still schedule out this task), If an other higher priority task becomes ready, context switch will remain pended until scheduler is unlocked.
Implemented by [scheduler.c](../../rtos/kernel/scheduler.c).

### scheduler\_unlock
This API will unlock the context of current task. This will invoke the scheduler if the system has missed a scheduling point. The context may be scheduled out if a higher priority task is now ready.
Implemented by [scheduler.c](../../rtos/kernel/scheduler.c).

### task\_create
This API initializes a task control block so that it can be added in the scheduler.
**takes** the task control block needed to be initialized.
**takes** the name for this task.
**takes** the task stack that will be used to run this task.
**takes** the task stack size.
**takes** the task entry function.
**takes** the arguments that will be passed to the task.
**takes** the flags for this task to set if this task might finish or not.
Implemented by [tasks.c](../../rtos/kernel/tasks.c).

### task\_yield
This API will try to yield current task. Task will only be switched if an higher or same priority task is found on the ready list.
Implemented by [kernel.c](../../rtos/kernel/kernel.c).

### kernel\_run
This API will start the system scheduler, this should be called once the system is initialized.
Implemented by [kernel.c](../../rtos/kernel/kernel.c).

### current\_system\_tick
This API will return the system clock's current tick. Application can use helper macros to convert this tick into actual time.
**returns** the current system tick.
Implemented by [kernel.c](../../rtos/kernel/kernel.c).

## Helper Macros
### SOFT\_TICKS\_PER\_SEC
This is the number of clock ticks per second.
Implemented by [kernel.h](../../rtos/kernel/kernel.h).

### MS\_TO\_TICK
This converts the given milliseconds to the number of equivalent system ticks.
**takes** number of milliseconds.
Implemented by [kernel.h](../../rtos/kernel/kernel.h).

### TICK\_TO\_MS
This converts the given ticks to the number of equivalent milliseconds.
**takes** number of ticks.
Implemented by [kernel.h](../../rtos/kernel/kernel.h).

### US\_TO\_HW\_TICK
This converts the given milliseconds to the number of equivalent hardware ticks.
**takes** number of milliseconds.
Implemented by [kernel.h](../../rtos/kernel/kernel.h).

### HW\_TICK\_TO\_US
This converts the given hardware ticks to the number of equivalent milliseconds.
**takes** number of hardware ticks.
Implemented by [kernel.h](../../rtos/kernel/kernel.h).

### ISR\_ENTER
This must be the first statement of an interrupt service routine.
Implemented by [kernel.h](../../rtos/kernel/kernel.h).

### ISR\_EXIT
This must be the last statement of an interrupt service routine.
Implemented by [kernel.h](../../rtos/kernel/kernel.h).

### KERNEL\_RUNNING
This returns true if the kernel scheduler is running.
Implemented by [kernel.h](../../rtos/kernel/kernel.h).

### get\_current\_task
This will return the control block of the the current task. Null will be returned if called from an interrupt service routine.
Implemented by [kernel.h](../../rtos/kernel/kernel.h).
