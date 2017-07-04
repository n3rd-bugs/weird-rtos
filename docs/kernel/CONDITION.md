Condition
=========
## Introduction
Weird RTOS provides a generalized suspension mechanism through *condition*. This not only generalize the suspensions in the system but also allows a task to suspend on multiple conditions to remove requirement of spawning separate handler tasks.

### Sources
- [condition.c](../../rtos/kernel/condition.c)

### Headers
- [condition.h](../../rtos/kernel/condition.h)

## Basic Concepts
A major feature that an OS provides is to allow tasks to wait for a certain condition and resume them when that condition is satisfied. A condition can be the availability of a semaphore, an external event or even data coming from a socket. Weird RTOS provides suspension services through *condition*. This is highly scalable and can be used to implement any kind of condition that an application might require.

### Basic features
- Common suspension flow for all components.
- Allows suspension for more than one conditions.
- Internally handles the protection of condition data through condition specific semaphore.
- Allows conditions to be triggered from context of an interrupt service routine.

### How it works
Weird RTOS breaks the suspension criteria in two parts

- *condition*  
As it's name suggest it defines the condition for suspension, e.g. condition to suspend on semaphore availability, condition to suspend on data availability on a socket etc. All the components that allow suspension provides APIs to query their condition.

- *suspend*  
This defines the suspension criteria. This mainly holds the task suspended on a condition and if we need to wait indefinitely or not.

So when a task wants to suspend on something it first create/initialize a condition for that, e.g. query semaphore for its condition or get data available condition for a socket or even create its own condition. Then it will initialize a suspend criteria to tell the OS how much it wants to wait on this condition. Once both are available it will suspend and start waiting for the condition to be satisfied. Once the OS detects that a condition is now satisfied it will resume this task.  
If an application needs to suspend on more than one conditions, it will first get all the conditions along with their suspend criteria and then create an array of pointers of these conditions and suspend criteria, and then suspend on them. Once resumed from a condition, OS will provide the index in the array for which task was resumed.

### Multiple tasks suspension
It is possible that more than one task suspend on same condition, in that case once the condition criteria is met, all the suspended tasks will be resumed at once. So the first highest priority task will be resumed at first as dictated by the scheduler.

## Data Structures
### CONDITION\_DO\_RESUME
This defines the resume callback to see if we need to resume a particular suspended task.

```
typedef uint8_t CONDITION_DO_RESUME (void *, void *);
```
### CONDITION\_DO\_SUSPEND
This defines the callback to see if a condition is now satisfied.

```
typedef uint8_t CONDITION_DO_SUSPEND (void *, void *);
```

### CONDITION\_LOCK
This is callback to lock access to condition and suspend data.

```
typedef void CONDITION_LOCK (void *);
```

### CONDITION\_UNLOCK
This is callback to unlock access to condition and suspend data.

```
typedef void CONDITION_UNLOCK (void *);
```

### RESUME
This defines the resume data.

```
typedef struct _resume
{
    /* Function that will be called to see if we need to resume. */
    CONDITION_DO_RESUME     *do_resume;

    void        *param;     /* User defined criteria. */
    int32_t     status;     /* Status needed to be returned to the task. */
} RESUME;
```

### SUSPEND
This defines the suspend data.

```
typedef struct _suspend
{
    /* Suspend link list member. */
    SUSPEND     *next;

    /* System tick at which we will resume this condition. */
    uint32_t    timeout;

    TASK        *task;      /* Task suspended on this. */
    void        *param;     /* User defined criteria for the tasks. */

    /* Flag to specify if the timer is enabled. */
    uint8_t     timeout_enabled;

    /* Structure padding. */
    uint8_t     pad[3];
} SUSPEND;
```

### CONDITION
This defines the condition data.

```
typedef struct _condition
{
    /* Link-list of the suspend on this condition. */
    SUSPEND_LIST            suspend_list;

    /* Function that will be called to see if we need to suspend. */
    CONDITION_DO_SUSPEND    *do_suspend;

    /* Function that will be called to get lock for condition. */
    CONDITION_LOCK          *lock;

    /* Function that will be called to release lock for this condition. */
    CONDITION_UNLOCK        *unlock;

    /* Private data that will be passed to the lock, unlock and suspend check
     * APIs.  */
    void                    *data;

    /* Condition flags. */
    uint32_t                flags;

} CONDITION;
```

## APIs
### suspend\_condition
This will suspend the caller task to wait for criteria(s).  
**takes** the array of conditions for which we need to suspend this task.  
**takes** the array of suspend criteria for each condition.  
**takes** the pointer to number of conditions we are waiting for. Can be null for one condition otherwise the index of the condition for which we resumed will be returned here.  
**takes** the flag to specify if the condition and suspend data is already locked or needed to be locked.  
**returns** success if a condition was satisfied, otherwise an error will be returned.  
Implemented by [condition.c](../../rtos/kernel/condition.c).

### resume\_condition
This will resume any tasks waiting for a condition.  
**takes** the condition for which we need to resume tasks.  
**takes** the resume data.  
**takes** the flag to specify if the condition and suspend data is already locked or needed to be locked.  
Implemented by [condition.c](../../rtos/kernel/condition.c).