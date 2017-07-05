/*
 * tasks.h
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
#ifndef _TASKS_H_
#define _TASKS_H_

#include <stdint.h>
#include <config.h>

/* Task configuration. */
//#define CONFIG_TASK_STATS
#define CONFIG_TASK_USAGE
#define CONFIG_STACK_PATTERN    'A'

/* These defines different task flags. */
#define TASK_NO_RETURN      0x01        /* This task will never return. */
#define TASK_SCHED_DRIFT    0x02        /* This task has caused scheduler to miss a tick. */

/* This is task entry function. */
typedef void TASK_ENTRY (void *argv);

/* This holds information about a single task. */
typedef struct _task TASK;
struct _task
{
#if (defined(CONFIG_TASK_STATS) && defined(CONFIG_TASK_USAGE))
    /* Number of ticks this task was scheduled. */
    uint64_t    total_active_ticks;

    /* Tick at which this task was last scheduled. */
    uint64_t    last_active_tick;
#endif /* (defined(CONFIG_TASK_STATS) && defined(CONFIG_TASK_USAGE)) */

    /* Task entry function. */
    TASK_ENTRY  *entry;

    /* Task arguments. */
    void        *argv;

    /* Task list member. */
    TASK        *next;

#ifdef CONFIG_TASK_STATS
    /* Number of times this task was scheduled. */
    uint32_t    scheduled;

    /* Global task list member. */
    TASK        *next_global;

    /* This is start of the stack pointer for this task. */
    uint8_t     *stack_start;

    /* Task stack size. */
    uint32_t    stack_size;

    /* Name for this task. */
    char        name[8];
#endif /* CONFIG_TASK_STATS */

    /* This holds current stack pointer of this task. */
    uint8_t     *tos;

    /* If suspended this will hold task suspension data. */
    void        *suspend_data;

    /* Number of wait conditions on which this task is waiting. */
    uint32_t    num_conditions;

#ifdef CONFIG_SLEEP
    /* Link list member for sleeping tasks. */
    TASK        *next_sleep;

    /* The system tick at which this task is needed to be rescheduled. */
    uint32_t    tick_sleep;
#endif /* CONFIG_SLEEP */

    /* This defines task priority. */
    uint8_t     priority;

    /* Current task status. */
    int32_t     status;

    /* Task flags as configured by scheduler. */
    uint8_t     flags;

    /* Lock count, how much nested scheduler locks have we acquired. */
    uint8_t     lock_count;

    /* Structure padding. */
    uint8_t     pad[1];
};

/* This defines a task list. */
typedef struct _task_list
{
    TASK        *head;
    TASK        *tail;
} TASK_LIST;

/* Function prototypes. */
void task_create(TASK *, char *, uint8_t *, uint32_t, TASK_ENTRY *, void *, uint8_t);

#endif /* _TASKS_H_ */
