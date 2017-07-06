/*
 * scheduler.h
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
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <kernel.h>

/* Scheduler lock configuration. */
#define SCHEDULER_MAX_LOCK          (5)

/* Defines the origin from which this task is being yielded.  */
#define YIELD_INIT                  (0x00)
#define YIELD_SYSTEM                (0x01)
#define YIELD_MANUAL                (0x02)
#define YIELD_SLEEP                 (0x03)

/* Some task resume status. */
#define TASK_SUSPENDED              (0)
#define TASK_RESUME                 (1)
#define TASK_RESUME_SLEEP           (2)
#define TASK_FINISHED               (3)

/* Global task list. */
#ifdef CONFIG_TASK_STATS
extern TASK_LIST sch_task_list;
#endif

/* Function prototypes. */
void scheduler_init();
void scheduler_task_add(TASK *, uint8_t);
void scheduler_task_remove(TASK *);
void scheduler_lock();
void scheduler_unlock();
TASK *scheduler_get_next_task();
void scheduler_task_yield(TASK *, uint8_t);

#endif /* _SCHEDULER_H_ */
