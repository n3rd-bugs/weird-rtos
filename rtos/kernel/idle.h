/*
 * idle.h
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

#ifndef _IDLE_H_
#define _IDLE_H_
#include <kernel.h>

/* Idle work configurations. */
#ifndef CMAKE_BUILD
//#define IDLE_RUNTIME_UPDATE
#define IDLE_WORK_MAX           (0)
#define IDLE_TASK_STACK_SIZE    (64)
#endif /* CMAKE_BUILD */

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
