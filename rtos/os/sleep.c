/*
 * sleep.c
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
#include <sleep.h>
#include <os.h>
#include <sll.h>

#ifdef CONFIG_SLEEP

/*
 * sleep_task_sort
 * @node: Existing task in the sleeping task list.
 * @task: New task being added in the sleeping task list.
 * @return: TRUE if the new task will be scheduled before the existing node
 *  otherwise FALSE will be returned.
 * This is sorting function called by SLL routines to sort the task list for
 * the sleeping tasks.
 */
static uint8_t sleep_task_sort(void *node, void *task)
{
    uint8_t schedule = FALSE;

    /* Check if task is needed to be scheduled before the given task . */
    if ( (((TASK *)node)->tick_sleep > ((TASK *)task)->tick_sleep) ||

         /* Check if this node task has same scheduling instance as the given task. */
         ( (((TASK *)node)->tick_sleep == ((TASK *)task)->tick_sleep) &&

           /* Schedule given task before the node if it has higher priority. */
           (((TASK *)node)->priority > ((TASK *)task)->priority) ) )
    {
        /* Schedule the given task before this node. */
        schedule = TRUE;
    }

    /* Return if we need to schedule this task before the given node. */
    return (schedule);

} /* sleep_task_sort. */

/*
 * sleep_process_system_tick
 * @return: If not NULL returns the task that is needed to be scheduled next.
 * This function is called by scheduler to see if there is a task that is needed
 * to wake-up at a given system tick.
 */
static TASK *sleep_process_system_tick(void)
{
    TASK *tcb = NULL;
    TASK *tcb_break = NULL;

    for (;;)
    {
        /* Check if we need to schedule a sleeping task. */
        if ( (sleep_scheduler.ready_tasks.head != tcb_break) &&
             (current_system_tick() >= sleep_scheduler.ready_tasks.head->tick_sleep) )
        {
            /* Schedule this sleeping task. */
            tcb = (TASK *)sll_pop(&sleep_scheduler.ready_tasks, OFFSETOF(TASK, next_sleep));

            /* Task is already resumed. */
            if (tcb->status != TASK_SUSPENDED)
            {
                /* Lets not get this task out of sleep any time soon. */
                tcb->tick_sleep = MAX_WAIT;

                /* Put back this task on the scheduler list at the end. */
                /* We will remove it from this list when we will resume. */
                sll_append(&sleep_scheduler.ready_tasks, tcb, OFFSETOF(TASK, next_sleep));

                /* Save the task at which we will need to break the search. */
                tcb_break = tcb;

                /* This is not the task we need. */
                tcb = NULL;

                /* Check if we can resume the next task. */
                continue;
            }
            else
            {
                /* Task is being resumed from sleep. */
                tcb->tick_sleep = 0;
                tcb->status = TASK_RESUME_SLEEP;
                tcb->flags |= TASK_RESUMED;
            }
        }

        break;
    }

    /* Return the task that is needed to be scheduled. */
    return (tcb);

} /* sleep_process_system_tick */

/*
 * sleep_task_re_enqueue
 * @tcb: Task's control block that is needed to be put back in the sleeping
 *  tasks list as there is a higher priority task that is needed to run before
 *  this task.
 * @from: From where this function was called.
 * This scheduler's yield function, for now this only called by scheduler API's
 * when a task is needed to put back in the scheduler list as there is an other
 * higher priority task.
 */
static void sleep_task_re_enqueue(TASK *tcb, uint8_t from)
{
    /* Process all the cases from a task can be re/scheduled. */
    switch (from)
    {
    /* Nothing to do here. */
    default:

        /* Remove some compiler warnings. */
        UNUSED_PARAM(tcb);

        break;
    }

} /* sleep_task_re_enqueue */

/*
 * sleep_add_to_list
 * @tcb: Task's control block that is needed to be added in the sleeping task's
 *  list.
 * @ticks: Number of ticks for which this task is needed to sleep.
 * This function is called when a task is needed to sleep for a particular
 * number of system ticks. This function adds the given task in the sleeping
 * tasks list. Interrupts must be locked by caller as sleep list can be
 * modified in the context of interrupts.
 */
void sleep_add_to_list(TASK *tcb, uint64_t ticks)
{
    /* Calculate system tick at which task will be invoked. */
    tcb->tick_sleep = current_system_tick() + ticks;

    /* Put this task on the list of sleeping tasks. */
    /* This will also schedule this task. */
    sll_insert(&sleep_scheduler.ready_tasks, tcb, &sleep_task_sort, OFFSETOF(TASK, next_sleep));

} /* sleep_add_to_list */

/*
 * sleep_remove_from_list
 * @tcb: Task's control block that is needed to be removed from the sleeping
 *  tasks list.
 * This function is called when a task is needed to be removed from the sleeping
 * tasks list.
 */
void sleep_remove_from_list(TASK *tcb)
{
    /* Remove this task from the list of sleeping tasks. */
    sll_remove(&sleep_scheduler.ready_tasks, tcb, OFFSETOF(TASK, next_sleep));

} /* sleep_remove_from_list */

/*
 * sleep_ticks
 * @ticks: Number of ticks for which this task is needed to sleep.
 * This function sleeps/suspends the current task for the given number of system
 * ticks.
 */
void sleep_ticks(uint32_t ticks)
{
    TASK *tcb;
    uint32_t interrupt_level;

    /* Save the current task pointer. */
    tcb = get_current_task();

    /* Current task should not be null. */
    OS_ASSERT(tcb == NULL);

    /* Interrupts must not be locked. */
    OS_ASSERT(tcb->interrupt_lock_count != 0);

    /* Lock the scheduler. */
    scheduler_lock();

    /* Disable interrupts. */
    interrupt_level = GET_INTERRUPT_LEVEL();
    DISABLE_INTERRUPTS();

    /* Add current task to the sleep list. */
    sleep_add_to_list(tcb, ticks);

    /* Task is being suspended. */
    tcb->status = TASK_SUSPENDED;

    /* Return control to the system.
     * We will resume from here when our required delay has been achieved. */
    CONTROL_TO_SYSTEM();

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Enable scheduling. */
    scheduler_unlock();

} /* sleep_ticks */

/*
 * sleep_hw_ticks
 * @ticks: Number of hardware ticks for which this task is needed to sleep.
 * This function will busy sleep the current task execution for given number
 * of hardware ticks.
 */
void sleep_hw_ticks(uint64_t ticks)
{
    uint64_t hw_tick;

    /* Wait before reading back from the LCD. */
    hw_tick = current_hardware_tick();
    while ((current_hardware_tick() - hw_tick) < ticks)
    {
        /* Yield current task to execute any high priority task. */
        task_yield();
    }

} /* sleep_hw_ticks */

/* This defines members for sleep scheduling class. */
SCHEDULER sleep_scheduler =
{
    /* List of tasks that are enqueued to run. */
    .ready_tasks    = {NULL, NULL},

    /* Function that will return the next task that is needed to run. */
    .get_task       = &sleep_process_system_tick,

    /* Function that will re-enqueue a given task. */
    .yield          = &sleep_task_re_enqueue,

    /* Priority for this scheduler, this should be less than aperiodic tasks. */
    .priority       = CONFIG_SLEEP_PIORITY
};

#endif /* CONFIG_SLEEP */
