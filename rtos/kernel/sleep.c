/*
 * sleep.c
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
#include <sleep.h>
#include <kernel.h>
#include <sll.h>

#ifdef CONFIG_SLEEP

/* Global variable definitions. */
static TASK_LIST sleep_task_list = {NULL, NULL};

/* This is used for time keeping in the system. */
uint32_t current_tick = 0;

/* Local function definitions. */
static uint8_t sleep_walk(void *node, void *param);

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
    if ( (INT32CMP(((TASK *)node)->tick_sleep, ((TASK *)task)->tick_sleep) > 0) ||

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

} /* sleep_task_sort */

/*
 * sleep_process_system_tick
 * This function is will be called at each system tick to see if we need to
 * resume any of the sleeping tasks.
 */
void sleep_process_system_tick(void)
{
    void *prev_node;
    TASK *tcb;
    uint8_t do_break = FALSE;

    /* While we have no task to resume. */
    while ((sleep_task_list.head) && (do_break == FALSE))
    {
        /* Walk the sleep task list and resume any tasks needed to be resumed. */
        tcb = sll_search(&sleep_task_list, &prev_node, &sleep_walk, &do_break, OFFSETOF(TASK, next_sleep));

        /* If we do have a task needed to be resumed. */
        if (do_break == FALSE)
        {
            /* If we have a task to resume. */
            if (tcb != NULL)
            {
                /* Remove this task from the sleep list. */
                sll_remove_node(&sleep_task_list, tcb, prev_node, OFFSETOF(TASK, next_sleep));

                /* Yield this task. */
                scheduler_task_yield(tcb, YIELD_SLEEP);
            }
            else
            {
                /* We have no more tasks to process. */
                break;
            }
        }
    }

} /* sleep_process_system_tick */

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
void sleep_add_to_list(TASK *tcb, uint32_t ticks)
{
    /* Calculate system tick at which task will be invoked. */
    tcb->tick_sleep = current_system_tick() + ticks;

    /* Put this task on the list of sleeping tasks. */
    /* This will also schedule this task. */
    sll_insert(&sleep_task_list, tcb, &sleep_task_sort, OFFSETOF(TASK, next_sleep));

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
    ASSERT(sll_remove(&sleep_task_list, tcb, OFFSETOF(TASK, next_sleep)) != tcb);

    /* Clear the sleep tick as we are just removed from the sleeping task list. */
    tcb->tick_sleep = 0;

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
    INT_LVL interrupt_level;

    /* Save the current task pointer. */
    tcb = get_current_task();

    /* Current task should not be null. */
    ASSERT(tcb == NULL);

    /* Lock the scheduler. */
    scheduler_lock();

    /* Add current task to the sleep list. */
    sleep_add_to_list(tcb, ticks);

    /* Disable interrupts. */
    interrupt_level = GET_INTERRUPT_LEVEL();
    DISABLE_INTERRUPTS();

    /* Task is being suspended. */
    tcb->state = TASK_TO_BE_SUSPENDED;

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
    while ((current_hardware_tick() - hw_tick) < ticks) ;

} /* sleep_hw_ticks */

/*
 * current_system_tick
 * @return: Current system tick.
 * This function returns the number of system ticks elapsed from the system
 * boot.
 */
uint32_t current_system_tick(void)
{
    uint32_t return_tick;
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Atomically save the current system tick. */
    return_tick = current_tick;

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Return current system tick. */
   return (return_tick);

} /* current_system_tick */

/*
 * process_system_tick
 * @return: TRUE if we need to trigger scheduler.
 * This function is called from tick source to update system tick.
 */
uint8_t process_system_tick(void)
{
    uint8_t trigger_scheduler = FALSE;

    /* Increment current tick. */
    current_tick++;

    /* If we need to schedule a context switch. */
    if ( (sleep_task_list.head != NULL) &&
         (INT32CMP(current_tick, sleep_task_list.head->tick_sleep) >= 0) )
    {
        /* Trigger scheduler. */
        trigger_scheduler = TRUE;
    }

    return (trigger_scheduler);

} /* process_system_tick */

/*
 * sleep_walk
 * @node: A task in the sleep list.
 * @do_break: Flag to specify if we need to break out of this loop.
 * This is match function to traverse sleep suspend list.
 */
static uint8_t sleep_walk(void *node, void *do_break)
{
    TASK *tcb = (TASK *)node;
    uint8_t matched = FALSE;

    /* If we need to resume this task. */
    if (INT32CMP(current_tick, tcb->tick_sleep) >= 0)
    {
        /* If task is in suspended state. */
        if (tcb->state == TASK_SUSPENDED)
        {
            /* Need to resume this task. */
            matched = TRUE;
        }
    }
    else
    {
        /* Sleep list is sorted, if we have a task that is needed to be
         * resumed in future, just break. */
        matched = TRUE;

        /* Break the loop. */
        *((uint8_t *)do_break) = TRUE;
    }

    /* Return if we have reached our criteria. */
    return (matched);

} /* sleep_walk */

#endif /* CONFIG_SLEEP */
