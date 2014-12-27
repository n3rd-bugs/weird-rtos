/*
 * scheduler.c
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
#include <scheduler.h>
#include <sll.h>
#include <sch_aperiodic.h>
#include <sch_periodic.h>
#include <sleep.h>
#include <string.h>

/* A list of all the tasks in the system. */
TASK_LIST sch_task_list;

/* This is the list of schedulers sorted on their priority. */
static SCHEDULER_LIST scheduler_list;

/* Definitions for idle task. */
static TASK __idle_task;
static char __idle_task_stack[128];

/*
 * __idle_task_entry
 * @argv: Unused parameter.
 * This is idle task that will run when there is no task in the system to run
 * at a particular instance.
 */
void __idle_task_entry(void *argv)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

    while(1)
    {
        ;
    }
}

/*
 * scheduler_sort
 * @node: Existing scheduling class in the list.
 * @scheduler: New scheduling class that is needed to be added in the list.
 * @return: TRUE if new scheduling class is needed to be placed before the given
 *  list node, otherwise FALSE will be returned.
 * This is sorting function called by SLL routines to sort scheduler list.
 */
static uint8_t scheduler_sort(void *node, void *scheduler)
{
    uint8_t higher_priority = FALSE;

    /* Check if scheduler has higher priority than the node. */
    if (((SCHEDULER *)node)->priority > ((SCHEDULER *)scheduler)->priority)
    {
        /* We need to insert the scheduler before the given node. */
        higher_priority = TRUE;
    }

    /* Return if scheduler is of higher priority than the given node. */
    return (higher_priority);

} /* scheduler_sort */

/*
 * scheduler_init
 * This function initializes scheduler, this must be called before adding any
 * tasks in the system.
 */
void scheduler_init()
{
    /* Clear the schedule lists. */
    memset(&scheduler_list, 0, sizeof(scheduler_list));
    memset(&sch_task_list, 0, sizeof(sch_task_list));

#ifdef CONFIG_INCLUDE_APERIODIC_TASKS
    /* Add aperiodic scheduler. */
    sll_insert(&scheduler_list, &aperiodic_scheduler, &scheduler_sort, OFFSETOF(SCHEDULER, next));
#endif /* CONFIG_INCLUDE_APERIODIC_TASKS */

#ifdef CONFIG_INCLUDE_PERIODIC_TASKS
    /* Add periodic scheduler. */
    sll_insert(&scheduler_list, &periodic_scheduler, &scheduler_sort, OFFSETOF(SCHEDULER, next));
#endif /* CONFIG_INCLUDE_PERIODIC_TASKS */

#ifdef CONFIG_INCLUDE_SLEEP
    /* Add scheduler for sleeping tasks. */
    sll_insert(&scheduler_list, &sleep_scheduler, &scheduler_sort, OFFSETOF(SCHEDULER, next));
#endif /* CONFIG_INCLUDE_SLEEP */

    /* Initialize idle task's control block and stack. */
    task_create(&__idle_task, "Idle", __idle_task_stack, 128, &__idle_task_entry, (void *)0x00);
    scheduler_task_add(&__idle_task, TASK_IDLE, 0, 0, 0);
} /* scheduler_init */

/*
 * scheduler_get_next_task
 * @return: Task's control block that will run next.
 * This function is called by system tick to get the next task that is needed to
 * run next.
 */
TASK *scheduler_get_next_task()
{
    SCHEDULER   *scheduler = scheduler_list.head;
    SCHEDULER   *scheduler_hp = scheduler;
    TASK        *tcb = NULL, *tcb_hp = NULL;

    /* Try to get a new task to run from registered schedulers. */
    while (scheduler != NULL)
    {
        /* Get the task that will run next on this scheduler. */
        tcb = scheduler->get_task();

        /* If this scheduler has a task to run. */
        if (tcb != NULL)
        {
            /* If we have not yet selected a task to run. */
            if ( (tcb_hp == NULL) ||

              /* If this scheduler has a higher priority task. */
              (tcb->priority < tcb_hp->priority) ||

              /* If this tasks have same priority but the scheduler has higher priority. */
              ( (tcb->priority == tcb_hp->priority) &&
                (scheduler->priority < scheduler_hp->priority) ) )
            {
                if (tcb_hp != NULL)
                {
                    /* Return the previously dequeued task to the scheduler. */
                    scheduler_hp->yield(tcb_hp, YIELD_CANNOT_RUN);
                }

                /* Run this task if possible. */
                tcb_hp = tcb;

                scheduler_hp = scheduler;
            }
            else
            {
                /* Put back this task to it's scheduler. */
                scheduler->yield(tcb, YIELD_CANNOT_RUN);
            }
        }

        /* Get the next scheduler class from the scheduler list. */
        scheduler = scheduler->next;
    }

    /* There is no task to run. */
    if (tcb_hp == NULL)
    {
        /* Just run the idle task. */
        tcb_hp = &__idle_task;
    }

#ifdef CONFIG_INCLUDE_TASK_STATS
    /* Increment the number of times this task was scheduled. */
    tcb_hp->scheduled ++;
#endif /* CONFIG_INCLUDE_TASK_STATS */

    /* Return the task to run. */
    return (tcb_hp);

} /* scheduler_get_next_task */

/*
 * scheduler_task_add
 * @tcb: Task control block that is needed to be added in the system.
 * @class: Scheduling class that is needed to be used for this task.
 * @priority: Priority for this task.
 * @param: Scheduling parameter if any.
 *  In case of periodic task this defines the task period in system ticks.
 * @flags: Task flags if any.
 * This function adds a task in the system, the task must be initialized before
 * adding.
 */
void scheduler_task_add(TASK *tcb, uint8_t class, uint32_t priority, uint64_t param, uint8_t flags)
{
    /* Get the first scheduler from the scheduler list. */
    SCHEDULER *scheduler = scheduler_list.head;

    /* Try to find the scheduler for which this task is being added. */
    while (scheduler != NULL)
    {
        if (scheduler->class == class)
        {
            /* Update the task control block. */
            tcb->scheduler          = scheduler;
            tcb->class              = class;
            tcb->priority           = priority;
            tcb->scheduler_data_1   = param;
            tcb->flags              = flags;

            /* Enqueue this task in the required scheduler. */
            scheduler->yield(tcb, YIELD_INIT);

            /* Break out of this loop. */
            break;
        }
        else
        {
            /* Get the next scheduler class from the list. */
            scheduler = scheduler->next;
        }
    }

#ifdef CONFIG_INCLUDE_TASK_STATS
    /* Append this task to the global task list. */
    sll_append(&sch_task_list, tcb, OFFSETOF(TASK, next_global));
#endif /* CONFIG_INCLUDE_TASK_STATS */

} /* scheduler_task_add */