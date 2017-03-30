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
 * @count: Initial count to be set for this semaphore.
 * @max_count: Maximum count for this semaphore.
 * @type: Type flags for this semaphore.
 *  SEMAPHORE_INT: This semaphore will also be accessed by interrupts.
 * This routine initializes a semaphore control block. After this the semaphore
 * can be used to protect shared resources.
 */
void semaphore_create(SEMAPHORE *semaphore, uint8_t count, uint8_t max_count, uint8_t type)
{
    /* Clear semaphore memory. */
    memset(semaphore, 0,  sizeof(SEMAPHORE));

    /* If this is interrupt accessible max count should be 1. */
    OS_ASSERT(((type & SEMAPHORE_INT) == TRUE) && (max_count != 1));

    /* Initialize semaphore count. */
    semaphore->count = count;
    semaphore->max_count = max_count;

    /* Initialize semaphore type. */
    semaphore->type = type;

    /* Initialize condition structure. */
    semaphore->condition.data = semaphore;

} /* semaphore_create */

/*
 * semaphore_update
 * @semaphore: Semaphore control block to be updated.
 * @count: New initial count to be set for this semaphore.
 * @max_count: New maximum count for this semaphore.
 * @type: Type flags for this semaphore.
 *  SEMAPHORE_INT: This semaphore will also be accessed by interrupts.
 * This routine will update the semaphore parameters.
 */
void semaphore_update(SEMAPHORE *semaphore, uint8_t count, uint8_t max_count, uint8_t type)
{
    /* Lock the scheduler. */
    scheduler_lock();

    /* Semaphore must not have been obtained. */
    OS_ASSERT(semaphore->count != semaphore->max_count);

    /* Update semaphore count. */
    semaphore->count = count;
    semaphore->max_count = max_count;

    /* Update semaphore type. */
    semaphore->type = type;

    /* Enable scheduling. */
    scheduler_unlock();

} /* semaphore_update */

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
    OS_ASSERT(semaphore->count != semaphore->max_count);
    OS_ASSERT((semaphore->type & SEMAPHORE_INT) == FALSE);

    /* Set semaphore interrupt data. */
    semaphore->interrupt_lock = lock;
    semaphore->interrupt_unlock = unlock;
    semaphore->interrupt_data = data;

    /* Enable scheduling. */
    scheduler_unlock();

} /* semaphore_update */

/*
 * semaphore_destroy
 * @semaphore: Semaphore control block to be destroyed.
 * This routine destroy a semaphore. If any of the tasks are waiting on this
 * semaphore they will be resumed with an error code.
 */
void semaphore_destroy(SEMAPHORE *semaphore)
{
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* No one should be waiting on this lock. */
    OS_ASSERT(semaphore->condition.suspend_list.head != NULL);

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
void semaphore_condition_get(SEMAPHORE *semaphore, CONDITION **condition, SUSPEND *suspend, uint64_t timeout)
{
    /* Initialize suspend criteria. */
    suspend->param = (void *)semaphore;
    suspend->do_suspend = &semaphore_do_suspend;

    /* If we don't want to wait indefinitely. */
    if (timeout != MAX_WAIT)
    {
        /* Calculate the tick at which we would want to be resumed. */
        suspend->timeout = current_system_tick() + timeout;
    }
    else
    {
        /* Wait indefinitely. */
        suspend->timeout = MAX_WAIT;
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
int32_t semaphore_obtain(SEMAPHORE *semaphore, uint64_t wait)
{
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();
    int32_t status = SUCCESS;
    SUSPEND suspend, *suspend_ptr = (&suspend);
    CONDITION *condition;
    TASK *tcb;

    /* Save the current task pointer. */
    tcb = get_current_task();

    /* Should never happen. */
    OS_ASSERT((tcb) && ((tcb->interrupt_lock_count > 0) && (semaphore->type & SEMAPHORE_INT)));

    /* Lock the scheduler. */
    scheduler_lock();

    /* If this is interrupt accessible semaphore. */
    if (semaphore->type & SEMAPHORE_INT)
    {
        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();
    }

    /* Check if this semaphore is not available. */
    if (semaphore->count == 0)
    {
        /* Should never happen. */
        OS_ASSERT((semaphore->type & SEMAPHORE_INT) && (semaphore->interrupt_lock == NULL));

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
        OS_ASSERT(semaphore->count == 0);

        /* Check if we have interrupt lock registered for this semaphore. */
        if (semaphore->type & SEMAPHORE_INT)
        {
            /* If we need to lock out specific interrupt. */
            if (semaphore->interrupt_lock != NULL)
            {
                /* Lock the required interrupt. */
                semaphore->interrupt_lock(semaphore->interrupt_data);

                /* Restore old interrupt level. */
                SET_INTERRUPT_LEVEL(interrupt_level);
            }
            else
            {
                /* If we are in a task. */
                if (tcb != NULL)
                {
                    /* Should never happen. */
                    OS_ASSERT(tcb->interrupt_lock_count == SCHEDULER_MAX_INT_LOCK);

                    /* Increment the interrupt lock count. */
                    tcb->interrupt_lock_count++;
                }

                /* Save the interrupt level and scheduler state. */
                semaphore->int_status = interrupt_level;
            }
        }

        /* Save the owner for this semaphore. */
        semaphore->owner = tcb;

        /* Decrease the semaphore count. */
        semaphore->count --;
    }
    else
    {
        /* If this is interrupt accessible lock. */
        if (semaphore->type & SEMAPHORE_INT)
        {
            /* Restore old interrupt level. */
            SET_INTERRUPT_LEVEL(interrupt_level);
        }
    }

    /* If this is not an interrupt accessible semaphore or we locked specific
     * interrupt. */
    if ((status != SUCCESS) || ((semaphore->type & SEMAPHORE_INT) == 0) || (semaphore->interrupt_lock != NULL))
    {
        /* Enable scheduling. */
        scheduler_unlock();
    }

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
    TASK *tcb = get_current_task();

    /* Semaphore double release. */
    OS_ASSERT(semaphore->count >= semaphore->max_count);

    /* If this is not an interrupt accessible semaphore or we locked specific
     * interrupt. */
    if (((semaphore->type & SEMAPHORE_INT) == 0) || (semaphore->interrupt_lock != NULL))
    {
        /* Lock the scheduler. */
        scheduler_lock();
    }

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
    if (semaphore->type & SEMAPHORE_INT)
    {
        /* Check if we have interrupt unlock registered for this semaphore. */
        if (semaphore->interrupt_unlock != NULL)
        {
            /* Unlock the required interrupt. */
            semaphore->interrupt_unlock(semaphore->interrupt_data);
        }
        else
        {
            /* If we have a task. */
            if (tcb != NULL)
            {
                /* Should never happen. */
                OS_ASSERT(tcb->interrupt_lock_count == 0);

                /* Decrement the interrupt lock count. */
                tcb->interrupt_lock_count--;
            }

            /* If we are not suspending. */
            if ((tcb == NULL) || ((tcb->status != TASK_WILL_SUSPENDED) && (tcb->interrupt_lock_count == 0)))
            {
                /* Restore the interrupt level. */
                SET_INTERRUPT_LEVEL(semaphore->int_status);
            }
        }
    }

    /* Enable scheduling. */
    scheduler_unlock();

} /* semaphore_release */

#endif /* CONFIG_SEMAPHORE */
