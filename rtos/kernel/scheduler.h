/*
 * scheduler.h
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
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <kernel.h>

/* Scheduler lock configuration. */
#ifndef CMAKE_BUILD
#define SCHEDULER_MAX_LOCK          (5)
#endif /* CMAKE_BUILD */

/* Defines the origin from which this task is being yielded.  */
#define YIELD_INIT                  (0x00)
#define YIELD_SYSTEM                (0x01)
#define YIELD_MANUAL                (0x02)
#define YIELD_SLEEP                 (0x03)

/* Global task list. */
#ifdef TASK_STATS
extern TASK_LIST sch_task_list;
#endif

/* Function prototypes. */
void scheduler_init(void);
void scheduler_task_add(TASK *, uint8_t);
void scheduler_task_remove(TASK *);
void scheduler_lock(void);
void scheduler_unlock(void);
TASK *scheduler_get_next_task(void);
void scheduler_task_yield(TASK *, uint8_t);

#endif /* _SCHEDULER_H_ */
