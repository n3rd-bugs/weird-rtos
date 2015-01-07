/*
 * semaphore.c
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
#include <os.h>
#include <sleep.h>
#include <sll.h>
#include <string.h>

#ifdef CONFIG_SEMAPHORE

/*
 * semaphore_create
 * @semaphore: Semaphore control block to be initialized.
 * @count: Initial count to be set for this semaphore.
 * @max_count: Maximum count for this semaphore.
 * This routine initializes a semaphore control block. After this the semaphore
 * can be used to protect important resources.
 */
void semaphore_create(SEMAPHORE *semaphore, uint8_t count, uint8_t max_count, uint8_t type)
{
    /* Clear semaphore memory. */
    memset(semaphore, 0,  sizeof(SEMAPHORE));

    /* Initialize semaphore count. */
    semaphore->count = count;
    semaphore->max_count = max_count;

    /* Initialize semaphore type. */
    semaphore->type = type;

    /* Initialize task list for this semaphore. */
    semaphore->tasks.head = NULL;
    semaphore->tasks.tail = NULL;

} /* semaphore_create */

/*
 * semaphore_fifo_sort
 * @node: Existing task in the semaphore task list.
 * @task: New task being added in the semaphore task list.
 * @return: TRUE if the new task is needed be scheduled before the existing node
 *  in the list.
 * This is sorting function called by SLL routines to sort the task list for
 * the FIFO semaphores.
 */
static uint8_t semaphore_fifo_sort(void *node, void *task)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(node);
    UNUSED_PARAM(task);

    /* Always return false so that the new task is placed at the end
     * of the list. */
    return (FALSE);

} /* semaphore_fifo_sort. */

/*
 * semaphore_priority_sort
 * @node: Existing task in the semaphore task list.
 * @task: New task being added in the semaphore task list.
 * @return: TRUE if the new task is needed be scheduled before the existing node
 *  in the list.
 * This is sorting function called by SLL routines to sort the task list for
 * the priority based semaphores.
 */
static uint8_t semaphore_priority_sort(void *node, void *task)
{
    uint8_t schedule = FALSE;

    /* Check f this node has lower priority than the new task. */
    if (((TASK *)node)->priority > ((TASK *)task)->priority)
    {
        /* Schedule the new task before this node. */
        schedule = TRUE;
    }

    /* Return if we need to schedule this task before the given node. */
    return (schedule);

} /* semaphore_priority_sort. */

/*
 * semaphore_obtain
 * @semaphore: Semaphore control block that is needed to be acquired.
 * @wait: The number of ticks to wait for this semaphore, MAX_WAIT should be
 *  used if user wants to wait for infinite time for this semaphore.
 * @return: SUCCESS if the semaphore was successfully acquired, SEMAPHORE_BUSY
 *  if the semaphore is busy and cannot be acquired, SEMAPHORE_TIMEOUT if system
 *  has exhausted the given timeout to obtain this semaphore.
 * This function is called to acquire a semaphore. User can specify the number
 * of ticks to wait before returning an error.
 */
uint32_t semaphore_obtain(SEMAPHORE *semaphore, uint32_t wait)
{
    uint32_t    status = SUCCESS;
    TASK        *tcb;

    /* Lock the scheduler. */
    scheduler_lock();

    /* Check if this semaphore is not available. */
    if (semaphore->count == 0)
    {
        /* Check if we need to wait for semaphore to be free. */
        if (wait > 0)
        {
            /* Save the current task pointer. */
            tcb = get_current_task();

#ifdef CONFIG_SLEEP
            /* Check if we need to wait for a finite time. */
            if (wait != (uint32_t)(MAX_WAIT))
            {
                /* Add the current task to the sleep list, if not available in
                 * the allowed time the task will be resumed. */
                sleep_add_to_list(tcb, wait);
            }
#endif /* CONFIG_SLEEP */

            /* If this is a FIFO semaphore. */
            if (semaphore->type == SEMAPHORE_FIFO)
            {
                /* Add this task on the semaphore's task list. */
                sll_insert(&semaphore->tasks, tcb, &semaphore_fifo_sort, OFFSETOF(TASK, next));
            }

            /* If this is a priority based semaphore. */
            else if (semaphore->type == SEMAPHORE_PRIORITY)
            {
                /* Add this task on the semaphore's task list. */
                sll_insert(&semaphore->tasks, tcb, &semaphore_priority_sort, OFFSETOF(TASK, next));
            }

            /* Task is being suspended. */
            tcb->status = TASK_SUSPENDED;

            /* Suspend and wait for being resumed by either semaphore
             * availability or wait timeout. */
            task_waiting();

            /* Check if we are resumed due to a timeout. */
            if (tcb->status == TASK_RESUME_SLEEP)
            {
                /* Return an error that we failed to get the semaphore in the
                 * given timeout. */
                status = SEMAPHORE_TIMEOUT;

                /* Remove this task from the semaphore's task's list. */
                sll_remove(&semaphore->tasks, tcb, OFFSETOF(TASK, next));
            }
        }

        /* We are not waiting for this semaphore to be free. */
        else
        {
            /* Return error to the caller. */
            status = SEMAPHORE_BUSY;
        }
    }

    if (status == SUCCESS)
    {
        /* Check if this semaphore is available. */
        if (semaphore->count > 0)
        {
            /* Decrease the semaphore count. */
            semaphore->count --;
        }

        /* We should never get here, if do return an error. */
        else
        {
            /* Return error to the caller. */
            status = SEMAPHORE_BUSY;
        }
    }

    /* Enable scheduling. */
    scheduler_unlock();

    /* Return status to the caller. */
    return (status);

} /* semaphore_obtain */

/*
 * semaphore_release
 * @semaphore: Semaphore that is needed to be released.
 * This function releases a previously acquired semaphore.
 */
void semaphore_release(SEMAPHORE *semaphore)
{
    TASK        *tcb;

    /* Lock the scheduler. */
    scheduler_lock();

    /* Semaphore double release. */
    OS_ASSERT(semaphore->count >= semaphore->max_count);

    /* Increment the semaphore count. */
    if (semaphore->count < semaphore->max_count)
    {
        semaphore->count ++;
    }

    /* Get the first task that can be resumed now. */
    tcb = (TASK *)sll_pop(&semaphore->tasks, OFFSETOF(TASK, next));

    if (tcb != NULL)
    {
        /* Task is resuming from a semaphore. */
        tcb->status = TASK_RESUME_SEMAPHORE;

#ifdef CONFIG_SLEEP
        /* Remove this task from sleeping tasks. */
        sleep_remove_from_list(tcb);
#endif /* CONFIG_SLEEP */

        /* Try to reschedule this task. */
        ((SCHEDULER *)(tcb->scheduler))->yield(tcb, YIELD_SYSTEM);

        /* Yield the current task and schedule the new task if required. */
        task_yield();
    }

    /* Enable scheduling. */
    scheduler_unlock();

} /* semaphore_release */

#endif /* CONFIG_SEMAPHORE */
