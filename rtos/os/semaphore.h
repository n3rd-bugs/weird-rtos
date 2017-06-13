/*
 * semaphore.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <os.h>

/* Interrupt protected lock structure. */
typedef volatile uint8_t INTLCK;

/* Initializes an interrupt protected lock. */
#define INTLCK_INIT(lock)                           \
    {                                               \
        uint32_t int_status = GET_INTERRUPT_LEVEL();\
        DISABLE_INTERRUPTS();                       \
        lock = 0;                                   \
        SET_INTERRUPT_LEVEL(int_status);            \
    }

/* Try to acquire an interrupt protected lock. */
#define INTLCK_TRY_GET(lock, acquired)              \
    {                                               \
        uint32_t int_status = GET_INTERRUPT_LEVEL();\
        acquired = FALSE;                           \
        DISABLE_INTERRUPTS();                       \
        if (lock == 0)                              \
        {                                           \
            lock++;                                 \
            SET_INTERRUPT_LEVEL(int_status);        \
            acquired = TRUE;                        \
        }                                           \
        else                                        \
            SET_INTERRUPT_LEVEL(int_status);        \
    }

/* Release an interrupt protected lock. */
#define INTLCK_RELEASE(lock)                        \
    {                                               \
        uint32_t int_status = GET_INTERRUPT_LEVEL();\
        DISABLE_INTERRUPTS();                       \
        lock--;                                     \
        SET_INTERRUPT_LEVEL(int_status);            \
    }

#ifdef CONFIG_SEMAPHORE
#include <condition.h>

/* Some status definitions. */
#define SEMAPHORE_BUSY          -700
#define SEMAPHORE_DELETED       -701

/* Semaphore interrupt lock/unlock APIs. */
typedef void SEM_INT_LOCK (void *);
typedef void SEM_INT_UNLOCK (void *);

/* Semaphore resume parameter. */
typedef struct _semaphore_param
{
    uint32_t    num;    /* Maximum number of tasks to resume. */
} SEMAPHORE_PARAM;

/* Semaphore data structure. */
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

/* Function prototypes. */
void semaphore_create(SEMAPHORE *, uint8_t, uint8_t, uint8_t);
void semaphore_update(SEMAPHORE *, uint8_t, uint8_t, uint8_t);
void semaphore_set_interrupt_data(SEMAPHORE *, void *, SEM_INT_LOCK *, SEM_INT_UNLOCK *);
void semaphore_destroy(SEMAPHORE *);
int32_t semaphore_obtain(SEMAPHORE *, uint32_t);
void semaphore_release(SEMAPHORE *);

/* Semaphore condition APIs. */
void semaphore_condition_get(SEMAPHORE *, CONDITION **, SUSPEND *, uint32_t);

#endif /* CONFIG_SEMAPHORE */
#endif /* _SEMAPHORE_H_ */
