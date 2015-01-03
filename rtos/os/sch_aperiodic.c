/*
 * sch_aperiodic.c
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
#include <sch_aperiodic.h>
#include <sll.h>

#ifdef CONFIG_APERIODIC_TASKS

/*
 * sch_aperiodic_task_sort
 * @node: An existing node in the list.
 * @task: New task that is needed to be added in the list.
 * @return: If the new task is needed to be scheduled before the existing node
 *  TRUE will be returned otherwise FALSE will be returned.
 * This is task sorting function that is used by SLL routines to schedule new
 * aperiodic tasks.
 */
static uint8_t sch_aperiodic_task_sort(void *node, void *task)
{
    uint8_t schedule = FALSE;

    /* If node has low priority than the given task, then we can schedule the
     * given task here. */
    if (((TASK *)node)->priority > ((TASK *)task)->priority)
    {
        /* Schedule the given task before this node. */
        schedule = TRUE;
    }

    /* Return if we need to schedule this task before the given node. */
    return (schedule);

} /* sch_aperiodic_task_sort */

/*
 * sch_aperiodic_task_yield
 * @tcb: The task's control block that is needed to be scheduled in the
 *  aperiodic scheduler.
 * @from: From where this task is being scheduled.
 * This is yield function required by a scheduling class, this is called when a
 * task is needed to be scheduled in the aperiodic scheduler.
 */
static void sch_aperiodic_task_yield(TASK *tcb, uint8_t from)
{
    /* Process all the cases from a task can be re/scheduled. */
    switch (from)
    {
    case YIELD_INIT:
    case YIELD_MANUAL:
    case YIELD_SYSTEM:
        /* Schedule the task being yielded/re-enqueued. */
        sll_insert(&aperiodic_scheduler.ready_tasks, tcb, &sch_aperiodic_task_sort, OFFSETOF(TASK, next));

        /* Task is being suspended. */
        tcb->status = TASK_SUSPENDED;

        break;

    case YIELD_CANNOT_RUN:
        /* Just put back this task on the scheduler list. */
        sll_push(&aperiodic_scheduler.ready_tasks, tcb, OFFSETOF(TASK, next));

        /* Task is being suspended. */
        tcb->status = TASK_SUSPENDED;

        break;

    default:
        break;
    }

} /* sch_aperiodic_task_yield */

/*
 * sch_aperiodic_get_task
 * @return: Task's control block which is needed to be run from this scheduler.
 * This function implements get task routine required by a scheduling class.
 * This is called by scheduler to get the next task that is needed to run.
 */
static TASK *sch_aperiodic_get_task()
{
    /* Get the first task in the ready list. */
    TASK *tcb = (TASK *)sll_pop(&aperiodic_scheduler.ready_tasks, OFFSETOF(TASK, next));

    /* If there is a task that can run next. */
    if (tcb != NULL)
    {
        /* Task is being resumed. */
        tcb->status = TASK_RESUME_NORMAL;
    }

    /* Return the task to be scheduled. */
    return (tcb);

} /* sch_aperiodic_get_task */

/* This defines members for aperiodic scheduling class. */
SCHEDULER aperiodic_scheduler =
{
    /* List of tasks that are enqueued to run. */
    .ready_tasks    = {NULL, NULL},

    /* Function that will return the next task that is needed to run. */
    .get_task       = &sch_aperiodic_get_task,

    /* Function that will yield/re-enqueue a given task. */
    .yield          = &sch_aperiodic_task_yield,

    /* Priority for this scheduler, this must be minimal as in normal cases
     * there will always be a task that is needed to be run. */
    .priority       = CONFIG_APERIODIC_PIORITY,

    /* ID specific for this class. */
    .class          = TASK_APERIODIC
};

#endif /* CONFIG_APERIODIC_TASKS */
