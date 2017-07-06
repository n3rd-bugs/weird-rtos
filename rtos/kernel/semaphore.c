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
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#include <kernel.h>

#ifdef CONFIG_SEMAPHORE
#include <sleep.h>
#include <sll.h>
#include <string.h>
#include <semaphore.h>

/* Internal function prototypes. */
static uint8_t semaphore_do_suspend(void *, void *);
static uint8_t semaphore_do_resume(void *, void *);

/*
 * semaphore_create
 * @semaphore: Semaphore control block to be initialized.
 * @max_count: Maximum count for this semaphore.
 * This routine initializes a semaphore control block. After this the semaphore
 * can be used to protect shared resources.
 */
void semaphore_create(SEMAPHORE *semaphore, uint8_t max_count)
{
    /* Clear semaphore memory. */
    memset(semaphore, 0,  sizeof(SEMAPHORE));

    /* Initialize semaphore count. */
    semaphore->count = semaphore->max_count = max_count;

    /* Initialize condition structure. */
    semaphore->condition.data = semaphore;
    semaphore->condition.do_suspend = &semaphore_do_suspend;

} /* semaphore_create */

/*
 * semaphore_set_interrupt_data
 * @semaphore: Semaphore for which interrupt lock/unlock data is needed to be updated.
 * @data: Data that will passed to interrupt lock/unlock APIs.
 * @lock: Function that will be called to lock interrupts.
 * @unlock: Function that will be called to unlock interrupts.
 * This routine will set interrupt manipulation APIs for a semaphore, this is
 * possible if a semaphore is required an access from an ISR, rather disabling
 * global interrupts, we can lock only the required interrupt.
 */
void semaphore_set_interrupt_data(SEMAPHORE *semaphore, void *data, SEM_INT_LOCK *lock, SEM_INT_UNLOCK *unlock)
{
    /* Lock the scheduler. */
    scheduler_lock();

    /* Semaphore must not have been obtained. */
    ASSERT(semaphore->count != semaphore->max_count);

    /* For interrupt accessible semaphore max count should be 1. */
    ASSERT(semaphore->max_count != 1);

    /* Mark this semaphore as interrupt protected. */
    semaphore->interrupt_protected = TRUE;

    /* Set semaphore interrupt data. */
    semaphore->interrupt_lock = lock;
    semaphore->interrupt_unlock = unlock;
    semaphore->interrupt_data = data;

    /* Enable scheduling. */
    scheduler_unlock();

} /* semaphore_set_interrupt_data */

/*
 * semaphore_destroy
 * @semaphore: Semaphore control block to be destroyed.
 * This routine destroy a semaphore. If any of the tasks are waiting on this
 * semaphore they will be resumed with an error code.
 */
void semaphore_destroy(SEMAPHORE *semaphore)
{
    RESUME resume;
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Resume any tasks waiting on this semaphore. */
    memset(&resume, 0, sizeof(RESUME));
    resume.status = SEMAPHORE_DELETED;

    /* Resume tasks waiting on this semaphore. */
    resume_condition(&semaphore->condition, &resume, TRUE);

    /* Clear the semaphore memory. */
    memset(semaphore, 0,  sizeof(SEMAPHORE));

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

} /* semaphore_destroy */

/*
 * semaphore_condition_get
 * @semaphore: Semaphore for which release condition is needed.
 * @condition: Pointer where condition will be returned.
 * @suspend: Suspend needed to be populated.
 * @timeout: Time to wait on this semaphore.
 * This function will return condition for release of this semaphore.
 */
void semaphore_condition_get(SEMAPHORE *semaphore, CONDITION **condition, SUSPEND *suspend, uint32_t timeout)
{
    /* Initialize suspend criteria. */
    suspend->param = (void *)semaphore;

    /* If we don't want to wait indefinitely. */
    if (timeout != MAX_WAIT)
    {
        /* Calculate the tick at which we would want to be resumed. */
        suspend->timeout = current_system_tick() + timeout;
        suspend->timeout_enabled = TRUE;
    }
    else
    {
        /* Wait indefinitely. */
        suspend->timeout_enabled = FALSE;
    }

    /* Return the condition for this semaphore. */
    *condition = &semaphore->condition;

} /* semaphore_condition_get */

/*
 * semaphore_do_suspend
 * @data: Condition data that will be used to access the semaphore.
 * @suspend_data: Suspend data, for now it is unused.
 * This function will called to see if we do need to suspend on a semaphore.
 */
static uint8_t semaphore_do_suspend(void *data, void *suspend_data)
{
    SEMAPHORE *semaphore = (SEMAPHORE *)data;
    uint8_t do_suspend = TRUE;

    /* For now unused. */
    UNUSED_PARAM(suspend_data != data);

    /* Check if semaphore is available. */
    if (semaphore->count > 0)
    {
        /* Don't need to suspend. */
        do_suspend = FALSE;
    }

    /* Return if we need to suspend or not. */
    return (do_suspend);

} /* semaphore_do_suspend */

/*
 * semaphore_do_resume
 * @param_resume: Parameter for which we need to resume a task.
 * @param_suspend: Parameter for which a task was suspended.
 * @return: TRUE if we need to resume this task, FALSE if we cannot resume
 *  this task.
 * This is callback to see if we can resume a task suspended on a semaphore.
 */
static uint8_t semaphore_do_resume(void *param_resume, void *param_suspend)
{
    uint8_t resume = FALSE;
    SEMAPHORE_PARAM *param = (SEMAPHORE_PARAM *)param_resume;

    /* Suspend criteria is unused. */
    UNUSED_PARAM(param_suspend);

    /* Check if the waiting task fulfills our criteria. */
    if (param->num > 0)
    {
        /* Decrement the number of tasks we need to resume. */
        param->num --;

        /* Resume this task. */
        resume = TRUE;
    }

    /* Return if we need to stop the search or need to process more. */
    return (resume);

} /* semaphore_do_resume */

/*
 * semaphore_obtain
 * @semaphore: Semaphore control block that is needed to be acquired.
 * @wait: The number of ticks to wait for this semaphore, MAX_WAIT should be
 *  used if user wants to wait for infinite time for this semaphore.
 * @return: SUCCESS if the semaphore was successfully acquired, CONDITION_TIMEOUT
 *  if the semaphore is busy and cannot be acquired, CONDITION_TIMEOUT if system
 *  has exhausted the given timeout to obtain this semaphore. SEMAPHORE_DELETED
 *  is returned if the given semaphore has been deleted.
 * This function is called to acquire a semaphore. User can specify the number
 * of ticks to wait before returning an error.
 */
int32_t semaphore_obtain(SEMAPHORE *semaphore, uint32_t wait)
{
    int32_t status = SUCCESS;
    SUSPEND suspend, *suspend_ptr = (&suspend);
    CONDITION *condition;
    TASK *tcb;
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();

    /* Should never happen. */
    ASSERT((semaphore->interrupt_protected == TRUE) && (semaphore->interrupt_lock == NULL));

    /* Save the current task pointer. */
    tcb = get_current_task();

    /* Lock the scheduler. */
    scheduler_lock();

    /* If this is interrupt accessible semaphore. */
    if (semaphore->interrupt_protected == TRUE)
    {
        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();
    }

    /* Check if this semaphore is not available. */
    if (semaphore->count == 0)
    {
        /* Check if we need to wait for semaphore to be free. */
        if ((wait > 0) && (tcb != NULL))
        {
            /* Initialize suspend condition for this semaphore. */
            semaphore_condition_get(semaphore, &condition, suspend_ptr, wait);

            /* Start waiting on this semaphore. */
            status = suspend_condition(&condition, &suspend_ptr, NULL, TRUE);
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
        /* Should never happen. */
        ASSERT(semaphore->count == 0);

        /* Check if we have interrupt lock registered for this semaphore. */
        if (semaphore->interrupt_protected == TRUE)
        {
            /* Lock the required interrupt. */
            semaphore->interrupt_lock(semaphore->interrupt_data);
        }

        /* Save the owner for this semaphore. */
        semaphore->owner = tcb;

        /* Decrease the semaphore count. */
        semaphore->count --;
    }

    /* If this is interrupt accessible lock. */
    if (semaphore->interrupt_protected == TRUE)
    {
        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);
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
    SEMAPHORE_PARAM param;
    RESUME resume;

    /* Semaphore double release. */
    ASSERT(semaphore->count >= semaphore->max_count);

    /* Lock the scheduler. */
    scheduler_lock();

    /* Increment the semaphore count. */
    semaphore->count ++;

    /* Clear the owner task. */
    semaphore->owner = NULL;

    /* Save the number of tasks we can resume. */
    param.num = semaphore->count;

    /* Initialize resume parameters. */
    resume.do_resume = &semaphore_do_resume;
    resume.param = &param;
    resume.status = TASK_RESUME;

    /* Resume tasks waiting on this semaphore. */
    resume_condition(&semaphore->condition, &resume, TRUE);

    /* If this is interrupt accessible semaphore. */
    if (semaphore->interrupt_protected == TRUE)
    {
        /* Unlock the required interrupt. */
        semaphore->interrupt_unlock(semaphore->interrupt_data);
    }

    /* Enable scheduling. */
    scheduler_unlock();

} /* semaphore_release */

#endif /* CONFIG_SEMAPHORE */
