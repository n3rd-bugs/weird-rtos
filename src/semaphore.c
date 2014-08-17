#include <semaphore.h>
#include <sleep.h>
#include <sll.h>

#ifdef CONFIG_INCLUDE_SEMAPHORE

void semaphore_create(SEMAPHORE *semaphore, uint8_t count, uint8_t max_count, uint8_t type)
{
    semaphore->count = count;
    semaphore->max_count = max_count;

    semaphore->type = type;

    semaphore->tasks.head = 0;
    semaphore->tasks.tail = 0;
}

static uint8_t semaphore_fifo_sort(void *node, void *task)
{
    /* Always return false so that the new task is placed at the end
     * of the list. */
    return (FALSE);

} /* semaphore_fifo_sort. */

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

uint32_t semaphore_obtain(SEMAPHORE *semaphore, uint32_t wait)
{
    uint32_t    status = SUCCESS;
    TASK        *tcb;

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Check if this semaphore is not available. */
    if (semaphore->count == 0)
    {
        /* Check if we need to wait for semaphore to be free. */
        if (wait > 0)
        {
            /* Save the current task pointer. */
            tcb = get_current_task();

            /* Check if we need to wait for a finite time. */
            if (wait != (uint32_t)(-1))
            {
                /* Add the current task to the sleep list, if not available in
                 * the allowed time the task will be resumed. */
                sleep_add_to_list(tcb, wait);
            }

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

            /* Disable global interrupts. */
            DISABLE_INTERRUPTS();

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

    /* Enable global interrupts. */
    ENABLE_INTERRUPTS();

    /* Return status to the caller. */
    return (status);

} /* semaphore_obtain */

void semaphore_release(SEMAPHORE *semaphore)
{
    TASK *tcb;

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Increment the semaphore count. */
    if (semaphore->count < semaphore->max_count)
    {
        semaphore->count ++;
    }

    /* Get the first task that can be resumed now. */
    tcb = (TASK *)sll_pop(&semaphore->tasks, OFFSETOF(TASK, next));

    if (tcb != 0)
    {
        /* Task is resuming from a semaphore. */
        tcb->status = TASK_RESUME_SEMAPHORE;

        /* Remove this task from sleeping tasks. */
        sleep_remove_from_list(tcb);

        /* Try to reschedule this task. */
        ((SCHEDULER *)(tcb->scheduler))->yield(tcb, YIELD_SYSTEM);

        /* Yield the current task and schedule the new task if required. */
        task_yield();
    }

    /* Enable global interrupts. */
    ENABLE_INTERRUPTS();

} /* semaphore_release */

#endif /* CONFIG_INCLUDE_SEMAPHORE */
