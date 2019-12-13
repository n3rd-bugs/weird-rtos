Idle
====
## Introduction
Weird RTOS requires an idle task in the system to return control to if there is no other task can run. It also allows application to offload any background work.

### Sources
- [idle.c](../../rtos/kernel/idle.c)

### Headers
- [idle.h](../../rtos/kernel/idle.h)

## Basic Concepts
When scheduler is initialized it adds an idle task in its scheduling table. This task is given the least priority of 255 and will only run when there is no other task available to run. As this is a requirement of the RTOS construct, it is extended to be more useful by allowing application to register their background work with it.
So when application initializes it can register a callback with some data, that is called continuously when idle task is active. It is to be noted the application should never try to suspend the idle task otherwise RTOS behaviour will be undefined.

## Configurations
### IDLE\_RUNTIME\_UPDATE
Configures if we want to add/remove work from idle task when scheduler is running. If enabled this will disable interrupts to synchronize the work list.

### IDLE\_WORK\_MAX
Configures the maximum number of idle works in the system. This can be set to 0 if user does not require idle work.

### IDLE\_TASK\_STACK\_SIZE
Configures the stack size of idle task.

## Data Structures
### IDLE\_DO
This defines the callback API to be called to perform background work.

```
typedef void (IDLE_DO) (void *);
```

## APIs
### idle\_task\_get
This API will return the control block of idle task.
**returns** the control block of idle task.
Implemented by [idle.c](../../rtos/kernel/idle.c).

### idle\_add\_work
This API will add a work to the idle task.
**takes** the work callback.
**takes** the data to be passed to the work callback.
**returns** success if idle work is successfully added, otherwise an error is returned.
Implemented by [idle.c](../../rtos/kernel/idle.c).

### idle\_remove\_work
This API will removes an existing work from the idle task.
**takes** the work callback to be unregistered.
**takes** the data associated with the work callback.
**returns** success if idle work is successfully removed, otherwise an error is returned.
Implemented by [idle.c](../../rtos/kernel/idle.c).