/*
 * tasks.h
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
#ifndef _TASKS_H_
#define _TASKS_H_

#include <stdint.h>
#include <config.h>
#include <tasks_config.h>

/* These defines different task flags. */
#define TASK_NO_RETURN              0x1     /* This task will never return. */
#define TASK_SCHED_DRIFT            0x2     /* This task has caused scheduler to miss a tick. */

/* Some task resume status. */
#define TASK_TO_BE_SUSPENDED        (0)     /* Task is being suspended indefinitely. */
#define TASK_SUSPENDED              (1)     /* Task is suspended indefinitely. */
#define TASK_RUNNING                (2)     /* Task is currently running. */
#define TASK_RESUME                 (3)     /* Task is to be resumed manually. */
#define TASK_SLEEP_RESUME           (4)     /* Task is to be resumed due to a timeout. */
#define TASK_FINISHED               (5)     /* Task is finished. */

/* Resume from definitions. */
#define TASK_RESUME_SYSTEM          (0)     /* Task was resumed manually. */
#ifdef CONFIG_SLEEP
#define TASK_RESUME_SLEEP           (1)     /* Task was resumed due to a timeout. */
#endif /* CONFIG_SLEEP */

/* This is task entry function. */
typedef void TASK_ENTRY (void *argv);

/* This holds information about a single task. */
typedef struct _task TASK;
struct _task
{
#if (defined(TASK_STATS) && defined(TASK_USAGE))
    /* Number of ticks this task was scheduled. */
    uint64_t    total_active_ticks;

    /* Tick at which this task was last scheduled. */
    uint64_t    last_active_tick;

#ifdef TASK_USAGE_RETAIN
    /* Number of ticks this task was scheduled in last reset. */
    uint64_t    last_total_active_ticks;
#endif /* TASK_USAGE_RETAIN */
#endif /* (defined(TASK_STATS) && defined(TASK_USAGE)) */

    /* Task entry function. */
    TASK_ENTRY  *entry;

    /* Task arguments. */
    void        *argv;

    /* Task list member. */
    TASK        *next;

#ifdef TASK_STATS
    /* Number of times this task was scheduled. */
    uint32_t    scheduled;

    /* Global task list member. */
    TASK        *next_global;

    /* This is start of the stack pointer for this task. */
    uint8_t     *stack_start;

    /* Task stack size. */
    uint32_t    stack_size;

    /* Name for this task. */
    P_STR_T     name;
#endif /* TASK_STATS */

    /* This holds current stack pointer of this task. */
    uint8_t     *tos;

    /* If suspended this will hold task suspension data. */
    void        *suspend_data;

#ifdef CONFIG_SLEEP
    /* Link list member for sleeping tasks. */
    TASK        *next_sleep;

    /* The system tick at which this task is needed to be rescheduled. */
    uint32_t    tick_sleep;
#endif /* CONFIG_SLEEP */

    /* Current task state. */
    uint8_t     state;

    /* Number of wait conditions on which this task is waiting. */
    uint8_t     num_conditions;

    /* This defines task priority. */
    uint8_t     priority;

    /* Task flags as configured by scheduler. */
    uint8_t     flags;

    /* Lock count, how much nested scheduler locks have we acquired. */
    uint8_t     lock_count;

    /* Holds the reference from where it was resumed. */
    uint8_t     resume_from;

    /* Structure padding. */
    uint8_t     pad[2];
};

/* This defines a task list. */
typedef struct _task_list
{
    TASK        *head;
    TASK        *tail;
} TASK_LIST;

/* Function prototypes. */
void task_create(TASK *, P_STR_T, uint8_t *, uint32_t, TASK_ENTRY *, void *, uint8_t);

#endif /* _TASKS_H_ */
