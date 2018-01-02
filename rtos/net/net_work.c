/*
 * net_work.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef CONFIG_NET
#include <net_work.h>
#ifdef CONFIG_SEMAPHORE
#include <semaphore.h>
#endif /* CONFIG_SEMAPHORE */
#include <string.h>
#include <sll.h>

/* Internal function prototypes. */
static void net_work_condition_process(void *, int32_t);
static void net_work_lock(void *);
static void net_work_unlock(void *);

/*
 * net_work_init
 * This function will initialize work queue.
 */
void net_work_init(WORK_QUEUE *work_queue)
{
    SYS_LOG_FUNCTION_ENTRY(NET_WORK);

    /* Clear work queue data. */
    memset(work_queue, 0, sizeof(WORK_QUEUE));

#ifdef CONFIG_SEMAPHORE
    /* Create lock to protect work queue. */
    semaphore_create(&work_queue->lock, 1);
#endif

    /* Initialize work queue condition. */
    work_queue->condition.data = work_queue;
    work_queue->condition.lock = &net_work_lock;
    work_queue->condition.unlock = &net_work_unlock;
    work_queue->suspend.timeout = MAX_WAIT;
    work_queue->suspend.timeout_enabled = FALSE;
    work_queue->suspend.priority = NET_USER_PRIORITY;
    work_queue->suspend.status = SUCCESS;

    /* Register work queue data with networking stack. */
    net_condition_add(&work_queue->condition, &work_queue->suspend, net_work_condition_process, work_queue);

    SYS_LOG_FUNCTION_EXIT(NET_WORK);

} /* net_work_init */

/*
 * net_work_add
 * @queue: Work queue in which we need to queue a new work.
 * @work: Work data, if null this function will allocation one on the stack.
 * @work_do: Work callback function.
 * @data: Data to be passed to work.
 * @wait: Number of ticks to wait for work to complete.
 * @return: Status returned by the work
 * This function is a callback function to process any queued work.
 */
int32_t net_work_add(WORK_QUEUE *queue, WORK *work, WORK_DO *work_do, void *data, uint32_t wait)
{
    int32_t status = SUCCESS;
    WORK work_int, *work_ptr = work;
    SUSPEND suspend, *suspend_ptr = (&suspend);
    CONDITION *condition;

    SYS_LOG_FUNCTION_ENTRY(NET_WORK);

    ASSERT(queue == NULL);
    ASSERT(work_do == NULL);

    /* If caller did not allocate the work. */
    if (work_ptr == NULL)
    {
        /* Use the local work pointer. */
        work_ptr = &work_int;
    }

    /* Initialize the work. */
    memset(work_ptr, 0, sizeof(WORK));
    work_ptr->data = data;
    work_ptr->work = work_do;

    /* Lock the work queue. */
    net_work_lock(queue);

    /* Add a new work on the work queue. */
    sll_push(&queue->list, work_ptr, OFFSETOF(WORK, next));

    /* Set the ping flag for work queue. */
    queue->condition.flags |= CONDITION_PING;

    /* Resume the worker task. */
    resume_condition(&queue->condition, NULL, TRUE);

    /* Release lock for work queue. */
    net_work_unlock(queue);

    /* If we need to wait for the work to finish. */
    if ((wait > 0) || (work_ptr == &work_int))
    {
        /* Lock the scheduler. */
        scheduler_lock();

        /* If we don't need to wait indefinitely. */
        if (wait != MAX_WAIT)
        {
            /* Set the required timeout. */
            suspend_ptr->timeout = current_system_tick() + wait;
            suspend_ptr->timeout_enabled = TRUE;
        }
        else
        {
            /* Disable the timer as we are waiting indefinitely. */
            suspend_ptr->timeout_enabled = FALSE;
        }

        /* Initialize suspend structure. */
        suspend_ptr->priority = NET_USER_PRIORITY;
        suspend_ptr->status = SUCCESS;

        /* Pick the condition on which we need to wait. */
        condition = &work_ptr->condition;

        /* Wait for this work to complete. */
        status = suspend_condition(&condition, &suspend_ptr, NULL, TRUE);

        /* If work did not complete successfully. */
        if (status != SUCCESS)
        {
            /* Return the status to the caller. */
            status = -(status);
        }

        /* Un-lock the scheduler. */
        scheduler_unlock();
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(NET_WORK, status);

    /* Return status to the caller. */
    return (status);

} /* net_work_add */

/*
 * net_work_lock
 * @wq: Work queue needed to be protected.
 * @timeout: Number of ticks we need to wait for the lock.
 * @return: Success will be returned if lock was successfully acquired.
 * This function will get the lock for work queue.
 */
static void net_work_lock(void *wq)
{
#ifdef CONFIG_SEMAPHORE
    /* Obtain lock for work queue data. */
    semaphore_obtain(&(((WORK_QUEUE *)wq)->lock), MAX_WAIT);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(timeout);

    /* Return success. */
    status = SUCCESS;

    /* Lock scheduler. */
    scheduler_lock();
#endif
} /* net_work_lock */

/*
 * net_work_unlock
 * @wq: Work queue for which lock is needed to released.
 * This function will release the lock for work queue.
 */
static void net_work_unlock(void *wq)
{
#ifdef CONFIG_SEMAPHORE
    /* Release lock for work queue data. */
    semaphore_release(&(((WORK_QUEUE *)wq)->lock));
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Enable scheduling. */
    scheduler_unlock();
#endif
} /* net_work_unlock */

/*
 * net_work_condition_process
 * @data: Work queue needed to be proceed.
 * @resume_status: Resumption status.
 * This function is a callback function to process any queued work.
 */
static void net_work_condition_process(void *data, int32_t resume_status)
{
    int32_t status;
    RESUME resume;
    WORK_QUEUE *work_queue = (WORK_QUEUE *)data;
    WORK *work;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(resume_status);

    SYS_LOG_FUNCTION_ENTRY(NET_WORK);

    do
    {
        /* Acquire lock for work queue. */
        net_work_lock(work_queue);

        /* Pop a work from the work list. */
        work = sll_pop(&work_queue->list, OFFSETOF(WORK, next));

        /* Release lock for work queue. */
        net_work_unlock(work_queue);

        /* If we do have a work to perform. */
        if (work != NULL)
        {
            /* Perform the required work. */
            status = work->work(work->data);

            /* Clear the resume structure. */
            memset(&resume, 0, sizeof(RESUME));

            /* If work did not compete successfully. */
            if (status != SUCCESS)
            {
                /* Return the status of the work to the caller, but
                 * always keep it positive. */
                resume.status = ((status > 0) ? status : -(status));
            }

            /* Lock the scheduler. */
            scheduler_lock();

            /* Ping any task waiting on completion of this work. */
            work->condition.flags |= CONDITION_PING;

            /* Execute the resume criteria. */
            resume_condition(&work->condition, &resume, TRUE);

            /* Un-lock the scheduler. */
            scheduler_unlock();
        }

      /* While we have some work to process. */
    } while (work != NULL);

    SYS_LOG_FUNCTION_EXIT(NET_WORK);

} /* net_work_condition_process */

#endif /* CONFIG_NET */
