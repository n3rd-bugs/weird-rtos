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
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <os.h>

#ifdef CONFIG_SEMAPHORE
#include <condition.h>

/* Some status definitions. */
#define SEMAPHORE_BUSY          -700
#define SEMAPHORE_DELETED       -701

/* Semaphore type flags. */
#define SEMAPHORE_INT           (0x01)

/* Semaphore interrupt lock/unlock APIs. */
typedef void SEM_INT_LOCK (void *data);
typedef void SEM_INT_UNLOCK (void *data);

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

    /* If semaphore is interrupt accessible this will store the interrupt
     * status when this semaphore was acquired. */
    uint32_t    int_status;

    /* Current semaphore count. */
    uint8_t     count;

    /* Maximum semaphore count there can be. */
    uint8_t     max_count;

    /* Type of this semaphore. */
    uint8_t     type;

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
