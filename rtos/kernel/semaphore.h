/*
 * semaphore.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <kernel.h>

/* Interrupt protected lock structure. */
typedef volatile uint8_t INTLCK;

/* Initializes an interrupt protected lock. */
#define INTLCK_INIT(lock)                           \
    {                                               \
        INT_LVL int_status = GET_INTERRUPT_LEVEL(); \
        DISABLE_INTERRUPTS();                       \
        lock = 0;                                   \
        SET_INTERRUPT_LEVEL(int_status);            \
    }

/* Try to acquire an interrupt protected lock. */
#define INTLCK_TRY_GET(lock, acquired)              \
    {                                               \
        INT_LVL int_status = GET_INTERRUPT_LEVEL(); \
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
        INT_LVL int_status = GET_INTERRUPT_LEVEL(); \
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
    /* Maximum number of tasks to resume. */
    uint8_t     num;

    /* Structure padding. */
    uint8_t     pad[3];

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
void semaphore_create(SEMAPHORE *, uint8_t);
void semaphore_set_interrupt_data(SEMAPHORE *, void *, SEM_INT_LOCK *, SEM_INT_UNLOCK *);
void semaphore_destroy(SEMAPHORE *);
int32_t semaphore_obtain(SEMAPHORE *, uint32_t);
void semaphore_release(SEMAPHORE *);

/* Semaphore condition APIs. */
void semaphore_condition_get(SEMAPHORE *, CONDITION **, SUSPEND *, uint32_t);

#endif /* CONFIG_SEMAPHORE */
#endif /* _SEMAPHORE_H_ */
