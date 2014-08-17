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
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _TASKS_H_
#define _TASKS_H_

#include <avr/io.h>

/* These defines different task flags. */
#define TASK_DONT_PREEMPT   0x01    /* This will disable task preemption. */

/* This holds information about a single task. */
typedef struct _task TASK;
struct _task
{
#ifdef CONFIG_INCLUDE_TASK_STATS
    /* Name for this task. */
    char        name[8];
#endif /* CONFIG_INCLUDE_TASK_STATS */

    /* Task list member. */
    TASK        *next;

#ifdef CONFIG_INCLUDE_SLEEP
    /* Link list member for sleeping tasks. */
    TASK        *next_sleep;
#endif

#ifdef CONFIG_INCLUDE_TASK_STATS
    /* Global task list member. */
    TASK        *next_global;
#endif /* CONFIG_INCLUDE_TASK_STATS */

    /* This holds current stack pointer of this task. */
    char        *tos;

     /* Task scheduling information. */
    void        *scheduler;

    /* If suspended this will hold task suspension data. */
    void        *suspend_data;

#ifdef CONFIG_INCLUDE_TASK_STATS
    /* This is start of the stack pointer for this task. */
    char        *stack_start;

    /* Task stack size. */
    uint64_t    scheduled;
#endif /* CONFIG_INCLUDE_TASK_STATS */

#ifdef CONFIG_INCLUDE_SLEEP
    /* The system tick at which this task is needed to be rescheduled. */
    uint64_t    tick_sleep;
#endif

    /* This holds scheduler parameters for this task. */
    uint64_t    scheduler_data_1;
    uint64_t    scheduler_data_2;

    /* This defines task priority. */
    uint32_t    priority;

    /* Current task status. */
    uint32_t    status;

#ifdef CONFIG_INCLUDE_TASK_STATS
    /* Task stack size. */
    uint32_t    stack_size;
#endif /* CONFIG_INCLUDE_TASK_STATS */

    /* Task scheduler class identifier. */
    uint8_t     class;

    /* Task flags as configured by scheduler. */
    uint8_t     flags;
};

/* This defines a task list. */
typedef struct _task_list
{
    TASK        *head;
    TASK        *tail;
} TASK_LIST;

/* This is task entry function. */
typedef void TASK_ENTRY (void *argv);

/* Function prototypes. */
void task_create(TASK *tcb, char *name, char *stack, uint32_t stack_size, TASK_ENTRY *entry, void *argv);

#endif /* _TASKS_H_ */
