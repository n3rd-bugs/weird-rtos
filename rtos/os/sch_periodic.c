/*
 * sch_periodic.c
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
#include <sch_periodic.h>
#include <sll.h>

#ifdef CONFIG_INCLUDE_PERIODIC_TASKS

/*
 * sch_periodic_task_sort
 * @node: An existing node in the list.
 * @task: New task that is needed to be added in the list.
 * @return: If the new task is needed to be scheduled before the existing node
 *  TRUE will be returned otherwise FALSE will be returned.
 * This is the task sorting routine that is used by SLL routines to schedule new
 * periodic tasks.
 */
static uint8_t sch_periodic_task_sort(void *node, void *task)
{
    uint8_t schedule = FALSE;

    /* If node has scheduling time greater than the given task then we need to
     * insert this task before this node. */
    if ( (((TASK *)node)->scheduler_data_2 > ((TASK *)task)->scheduler_data_2) ||

         /* If node has scheduling time equal to the given task but the node has
          * lass priority then we need to schedule this task before this node. */
         ( ((((TASK *)node)->scheduler_data_2 == ((TASK *)task)->scheduler_data_2)) &&
           (((TASK *)node)->priority > ((TASK *)task)->priority) ) )
    {
        /* Schedule the given task before this node. */
        schedule = TRUE;
    }

    /* Return if we need to schedule this task before the given node. */
    return (schedule);

} /* sch_periodic_task_sort */

/*
 * sch_periodic_task_yield
 * @tcb: The task's control block that is needed to be scheduled in the periodic
 *  scheduler.
 * @from: From where this task is being scheduled.
 * This is yield function required by a scheduling class, this is called when a
 * task is needed to be scheduled in the periodic scheduler.
 */
static void sch_periodic_task_yield(TASK *tcb, uint8_t from)
{
    uint64_t last_time;

    /* Process all the cases from a task can be re/scheduled. */
    switch (from)
    {
    case YIELD_CANNOT_RUN:
        /* Just put back this task on the scheduler list. */
        sll_push(&periodic_scheduler.ready_tasks, tcb, OFFSETOF(TASK, next));

        /* Task is being suspended. */
        tcb->status = TASK_SUSPENDED;

        break;

    case YIELD_INIT:
        /* At the time of initialization this will contain the tick at which this
         * task was registered. */
        tcb->scheduler_data_2 = current_system_tick();

        /* Schedule this task. */
        sll_insert(&periodic_scheduler.ready_tasks, tcb, &sch_periodic_task_sort, OFFSETOF(TASK, next));

        /* Task is being suspended. */
        tcb->status = TASK_SUSPENDED;

        break;

    case YIELD_SYSTEM:
        /* Save the tick at which this task was originally scheduled. */
        last_time = tcb->scheduler_data_2;

        /* Try to reschedule this task. */
        tcb->scheduler_data_2 = current_system_tick();
        sll_insert(&periodic_scheduler.ready_tasks, tcb, &sch_periodic_task_sort, OFFSETOF(TASK, next));

        /* For periodic tasks we don't want to lose original time period. */
        tcb->scheduler_data_2 = last_time;

        /* Task is being suspended. */
        tcb->status = TASK_SUSPENDED;

        break;

    case YIELD_MANUAL:
        /* Calculate the tick at which this task is needed to be rescheduled. */
        tcb->scheduler_data_2 += tcb->scheduler_data_1;

        /* Schedule the task being yielded/re-enqueued. */
        sll_insert(&periodic_scheduler.ready_tasks, tcb, &sch_periodic_task_sort, OFFSETOF(TASK, next));

        /* Task is being suspended. */
        tcb->status = TASK_SUSPENDED;

        break;

    default:
        break;
    }

} /* sch_periodic_task_yield */

/*
 * sch_periodic_get_task
 * @return: Task's control block which is needed to be run from this scheduler.
 * This function implements get task routine required by a scheduling class.
 * This is called by scheduler to get the next task that is needed to run.
 */
static TASK *sch_periodic_get_task()
{
    TASK *tcb = NULL;

    /* Check if we need to schedule the task on the head. */
    if ( (periodic_scheduler.ready_tasks.head != NULL) &&
         (current_system_tick() >= periodic_scheduler.ready_tasks.head->scheduler_data_2) )
    {
        tcb = (TASK *)sll_pop(&periodic_scheduler.ready_tasks, OFFSETOF(TASK, next));

        /* Task is being resumed. */
        tcb->status = TASK_RESUME_NORMAL;
    }

    /* Return the task that is needed to run next. */
    return (tcb);

} /* sch_periodic_get_task */

/* This defines members for aperiodic scheduling class. */
SCHEDULER periodic_scheduler =
{
    /* List of tasks that are enqueued to run. */
    .ready_tasks    = {NULL, NULL},

    /* Function that will return the next task that is needed to run. */
    .get_task       = &sch_periodic_get_task,

    /* Function that will yield/re-enqueue a given task. */
    .yield          = &sch_periodic_task_yield,

    /* Priority for this scheduler. */
    .priority       = CONFIG_PERIODIC_PIORITY,

    /* ID specific for this class. */
    .class          = TASK_PERIODIC
};

#endif /* CONFIG_INCLUDE_PERIODIC_TASKS */
