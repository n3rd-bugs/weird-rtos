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

#include "os.h"

#ifdef CONFIG_INCLUDE_SEMAPHORE

/* Some status definitions. */
#define SEMAPHORE_TIMEOUT   -700
#define SEMAPHORE_BUSY      -701

/* Semaphore type definitions. */
#define SEMAPHORE_FIFO      0x01
#define SEMAPHORE_PRIORITY  0x02

typedef struct _semaphore
{
    /* Lists of tasks waiting for this semaphore. */
    TASK_LIST   tasks;

    /* Current semaphore count. */
    uint8_t     count;

    /* Maximum semaphore count there can be. */
    uint8_t     max_count;

    /* Type of this semaphore. */
    uint8_t     type;

} SEMAPHORE;

/* Function prototypes. */
void semaphore_create(SEMAPHORE *semaphore, uint8_t count, uint8_t max_count, uint8_t type);
uint32_t semaphore_obtain(SEMAPHORE *semaphore, uint32_t wait);
void semaphore_release(SEMAPHORE *semaphore);

#endif /* CONFIG_INCLUDE_SEMAPHORE */

#endif /* _SEMAPHORE_H_ */
