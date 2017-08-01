/*
 * idle.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */

#ifndef _IDLE_H_
#define _IDLE_H_
#include <kernel.h>

/* Idle work configurations. */
//#define IDLE_RUNTIME_UPDATE
#define IDLE_WORK_MAX           (0)
#define IDLE_TASK_STACK_SIZE    (64)

/* Error definitions. */
#define IDLE_NO_SPACE           -500
#define IDLE_NOT_FOUND          -501
#define IDLE_CANNOT_UPDATE      -502

/* Idle work function definition. */
typedef void (IDLE_DO) (void *);

/* Idle work definition. */
typedef struct _idle_work
{
    /* Function to be called to do some work. */
    IDLE_DO     *do_fun;

    /* Private data associated to be passed to work routine. */
    void        *priv_data;

} IDLE_WORK;

/* Function prototypes. */
void idle_task_init(void);
TASK *idle_task_get(void);
int32_t idle_add_work(IDLE_DO *, void *);
int32_t idle_remove_work(IDLE_DO *, void *);

#endif /* _IDLE_H_ */
