Semaphore
=========
## Introduction
Semaphores in Weird RTOS are designed especially for the development of small embedded applications. Here are notable features

- Allows tasks to wait for a semaphore to become available.
- Can be used to protect resources that are not accessible in the context of interrupts.
- Can protect resources that can be accessed in the context of interrupts.
- Supports both counting and binary semaphores.

### Sources
- [semaphore.c](../../rtos/kernel/semaphore.c)

### Headers
- [semaphore.h](../../rtos/kernel/semaphore.h)

## Basic Concepts
A semaphore can be used in two modes
1. *Normal mode*  
In default mode the semaphore works like a normal lock, allows tasks to suspend on it. In such mode semaphore can only be accessed in the context of a task.
2. *Interrupt protected mode*  
It is possible that the resources semaphore is trying to protect is also to be accessed in the context of interrupt. This interrupt control APIs to be linked with a semaphore so that a specific interrupt can also be locked with semaphore.

Semaphore uses condition to implement task suspend and also allows application to extract suspend condition.

### Interrupt protected locks
In some use cases it is not possible to suspend the current task. For that interrupt protected locks can be used, they allow a user to busy wait for lock to become available. Please note that interrupt protected locks locks the interrupts when being acquired or released so application should use them with caution.

## Data Structures
### SEMAPHORE
This holds data associated to a semaphore

```
typedef struct _semaphore
{
    /* Semaphore condition structure. */
    CONDITION   condition;

    /* Current owner of this semaphore if any. */
    TASK        *owner;

    /* Interrupt manipulation APIs. */
    SEM_INT_LOCK    *interrupt_lock;
    SEM_INT_UNLOCK  *interrupt_unlock;
    void        *interrupt_data;

    /* Current semaphore count. */
    uint8_t     count;

    /* Maximum semaphore count there can be. */
    uint8_t     max_count;

    /* Flag to specify if this is an interrupt
     * protected lock. */
    uint8_t     interrupt_protected;

    /* Padding variable. */
    uint8_t     pad[1];

} SEMAPHORE;
```

### SEM\_INT\_LOCK
This defines callback API to lock interrupt associated with a semaphore.

```
typedef void SEM_INT_LOCK (void *);
```

### SEM\_INT\_UNLOCK
This defines callback API to unlock interrupt associated with a semaphore.

```
typedef void SEM_INT_UNLOCK (void *);
```

### INTLCK
This defines data structure for an interrupt protected lock.

```
typedef volatile uint8_t INTLCK;
```

## APIs
### semaphore\_create
This API initialize a semaphore that can be used later. Semaphore is always created as released.  
**takes** the control block of semaphore needed to be initialized.  
**takes** the maximum count for this semaphore, a binary semaphore will have a value of 1.  
Implemented by [semaphore.c](../../rtos/kernel/semaphore.c).

### semaphore\_set\_interrupt\_data
This will set interrupt data for this semaphore and also mark this semaphore to be accessible in the context of interrupt service routines. This operation cannot be reverted.  
**takes** the control block of semaphore needed to be updated.  
**takes** private data to be passed to interrupt control routines.  
**takes** callback to lock the interrupt.  
**takes** callback to unlock the interrupt.  
Implemented by [semaphore.c](../../rtos/kernel/semaphore.c).

### semaphore\_destroy
This API destroyers a semaphore. This will also resume any tasks waiting on this semaphore.  
**takes** the control block of semaphore needed to be destroyed.  
Implemented by [semaphore.c](../../rtos/kernel/semaphore.c).

### semaphore\_condition\_get
This API returns semaphore condition that can be used to wait for semaphore to become available.  
**takes** the control block of semaphore for which condition is required.  
**takes** the address at which the condition will be returned.  
**takes** the suspend criteria to be populated for this semaphore.  
**takes** the number of ticks to wait for semaphore, use MAX_WAIT to wait indefinitely.  
Implemented by [semaphore.c](../../rtos/kernel/semaphore.c).

### semaphore\_obtain
This API locks the given semaphore. If the caller is a task it can suspend and wait for the semaphore to become available. In case of this is a counting semaphore, the count will be incremented if available.  
**takes** the control block of semaphore to lock.  
**takes** the number of ticks to wait for semaphore, use MAX_WAIT to wait indefinitely.  
**returns** success if semaphore is successfully obtained otherwise an error will be returned.  
Implemented by [semaphore.c](../../rtos/kernel/semaphore.c).

### semaphore\_release
This API release the given semaphore. This will also resume any tasks wait for semaphore condition.  
**takes** the control block of semaphore to release.  
Implemented by [semaphore.c](../../rtos/kernel/semaphore.c).

## Helper Macros
### INTLCK\_INIT
Initialize the given interrupt protected lock.  
**takes** the control block of interrupt protected lock.  
Implemented by [semaphore.h](../../rtos/kernel/semaphore.h).

### INTLCK\_TRY\_GET
Tries to acquire the interrupt protected lock.  
**takes** the control block of interrupt protected lock needed to be locked.  
**takes** the variable to be set if the lock is successfully acquired and unset if not acquired.  
Implemented by [semaphore.h](../../rtos/kernel/semaphore.h).

### INTLCK\_RELEASE
Release the given interrupt protected lock.  
**takes** the control block of interrupt protected lock needed to be released.  
Implemented by [semaphore.h](../../rtos/kernel/semaphore.h).