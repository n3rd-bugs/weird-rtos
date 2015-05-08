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

#include <stdint.h>
#include <config.h>

/* Task configuration. */
#define CONFIG_TASK_STATS
#define CONFIG_STACK_PATTERN    'A'

/* These defines different task flags. */
#define TASK_NO_RETURN      0x01        /* This task will never return. */

/* This is task entry function. */
typedef void TASK_ENTRY (void *argv);

/* This holds information about a single task. */
typedef struct _task TASK;
struct _task
{
    /* This holds scheduler parameters for this task. */
    uint64_t    scheduler_data_1;
    uint64_t    scheduler_data_2;

#ifdef CONFIG_SLEEP
    /* The system tick at which this task is needed to be rescheduled. */
    uint64_t    tick_sleep;
#endif

#ifdef CONFIG_TASK_STATS
    /* Number of times this task was scheduled. */
    uint64_t    scheduled;
#endif /* CONFIG_TASK_STATS */

    /* Task entry function. */
    TASK_ENTRY  *entry;

    /* Task arguments. */
    void        *argv;

    /* Task list member. */
    TASK        *next;

#ifdef CONFIG_SLEEP
    /* Link list member for sleeping tasks. */
    TASK        *next_sleep;
#endif

#ifdef CONFIG_TASK_STATS
    /* Global task list member. */
    TASK        *next_global;
#endif /* CONFIG_TASK_STATS */

    /* This holds current stack pointer of this task. */
    uint8_t     *tos;

     /* Task scheduling information. */
    void        *scheduler;

    /* If suspended this will hold task suspension data. */
    void        *suspend_data;

#ifdef CONFIG_TASK_STATS
    /* This is start of the stack pointer for this task. */
    uint8_t     *stack_start;

    /* Name for this task. */
    char        name[8];

    /* This defines task priority. */
    uint32_t    priority;

#endif /* CONFIG_TASK_STATS */

    /* Current task status. */
    int32_t     status;

#ifdef CONFIG_TASK_STATS
    /* Task stack size. */
    uint32_t    stack_size;

#endif /* CONFIG_TASK_STATS */

    /* Task scheduler class identifier. */
    uint8_t     class;

    /* Task flags as configured by scheduler. */
    uint8_t     flags;

    /* Lock count, how much nested scheduler locks have we acquired. */
    uint8_t     lock_count;

    /* Lock count, how much nested IRQ locks have we acquired. */
    uint8_t     irq_lock_count;
#ifdef CONFIG_SLEEP

    /* Padding variable (needs to be 64-bit aligned). */
    uint8_t     pad2[4];
#endif
};

/* This defines a task list. */
typedef struct _task_list
{
    TASK        *head;
    TASK        *tail;
} TASK_LIST;

/* Function prototypes. */
void task_create(TASK *, char *, uint8_t *, uint32_t, TASK_ENTRY *, void *, uint8_t);
uint8_t task_priority_sort(void *, void *);

#endif /* _TASKS_H_ */
