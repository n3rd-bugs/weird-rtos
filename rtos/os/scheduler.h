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
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <os.h>

/* Scheduler configurations. */
#define CONFIG_APERIODIC_TASKS
#define CONFIG_PERIODIC_TASKS

/* Scheduler priority configurations. */
#define CONFIG_PERIODIC_PIORITY     0
#define CONFIG_SLEEP_PIORITY        254
#define CONFIG_APERIODIC_PIORITY    255

/* These defines different scheduler classes. */
#define TASK_APERIODIC              0x01
#define TASK_PERIODIC               0x02
#define TASK_IDLE                   0x03

/* Some task resume status. */
#define TASK_SUSPENDED              0
#define TASK_RESUME_NORMAL          1
#define TASK_RESUME_SLEEP           2
#define TASK_RESUME_SEMAPHORE       3

/* Scheduler class definition. */
typedef struct _scheduler SCHEDULER;
struct _scheduler
{
    /* This is scheduler list member. */
    SCHEDULER   *next;

    /* These are scheduler specific functions. */
    TASK        *(*get_task) (void);
    void        (*yield) (TASK *, uint8_t);

    /* Some internal members. */
    TASK_LIST    tasks;
    TASK_LIST    ready_tasks;

    /* Priority of this scheduling class. */
    uint8_t     priority;

    /* Scheduler class identifier. */
    uint8_t     class;
};

/* Scheduler list structure. */
typedef struct _scheduler_list
{
    SCHEDULER   *head;
    SCHEDULER   *tail;
} SCHEDULER_LIST;

/* Global task list. */
extern TASK_LIST sch_task_list;

/* Function prototypes. */
void scheduler_init();
TASK *scheduler_get_next_task();
void scheduler_task_add(TASK *tcb, uint8_t class, uint32_t priority, uint64_t param, uint8_t flags);

#endif /* _SCHEDULER_H_ */
