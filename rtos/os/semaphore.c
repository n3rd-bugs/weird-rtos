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
 *  SEMAPHORE_FIFO: FIFO semaphore task queue.
 *  SEMAPHORE_PRIORITY: Priority based task queue.
 *  SEMAPHORE_IRQ: This semaphore will also be accessed by IRQs.
 * This routine initializes a semaphore control block. After this the semaphore
 * can be used to protect important resources.
 */
void semaphore_create(SEMAPHORE *semaphore, uint8_t count, uint8_t max_count, uint8_t type)
{
    /* Clear semaphore memory. */
    memset(semaphore, 0,  sizeof(SEMAPHORE));

    /* If this is IRQ accessible than count should be zero. */
    OS_ASSERT((type == SEMAPHORE_IRQ) && (count != 0));

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
 *  SEMAPHORE_FIFO: FIFO semaphore task queue.
 *  SEMAPHORE_PRIORITY: Priority based task queue.
 *  SEMAPHORE_IRQ: This semaphore will also be accessed by IRQs.
 * This routine will update the semaphore parameters.
 */
void semaphore_update(SEMAPHORE *semaphore, uint8_t count, uint8_t max_count, uint8_t type)
{
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Semaphore must not have been obtained. */
    OS_ASSERT(semaphore->count != semaphore->max_count);

    /* Update semaphore count. */
    semaphore->count = count;
    semaphore->max_count = max_count;

    /* Update semaphore type. */
    semaphore->type = type;

    /* Restore the IRQ interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

} /* semaphore_update */

/*
 * semaphore_set_irq_data
 * @semaphore: Semaphore for which IRQ lock/unlock data is needed to be updated.
 * @data: Data that will passed to IRQ lock/unlock APIs.
 * @lock: Function that will be called to lock interrupts.
 * @unlock: Function that will be called to unlock interrupts.
 * This routine will set IRQ manipulation APIs for a semaphore, this is
 * possible if a semaphore is required an access from an ISR, rather disabling
 * global IRQ'a we can lock only the required IRQ.
 */
void semaphore_set_irq_data(SEMAPHORE *semaphore, void *data, SEM_IRQ_LOCK *lock, SEM_IRQ_UNLOCK *unlock)
{
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Semaphore must not have been obtained. */
    OS_ASSERT(semaphore->count != semaphore->max_count);
    OS_ASSERT((semaphore->type & SEMAPHORE_IRQ) == 0)

    /* Set semaphore IRQ data. */
    semaphore->irq_lock = lock;
    semaphore->irq_unlock = unlock;
    semaphore->irq_data = data;

    /* Restore the IRQ interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

} /* semaphore_update */

/*
 * semaphore_destroy
 * @semaphore: Semaphore control block to be destroyed.
 * This routine destroy a semaphore. If any of the tasks are waiting on this
 * semaphore they will be resumed with an error code.
 */
void semaphore_destroy(SEMAPHORE *semaphore)
{
    RESUME resume;

    /* Initialize resume data. */
    resume.do_resume = resume.param = NULL;
    resume.status = SEMAPHORE_DELETED;

    /* Resume all tasks waiting on this semaphore. */
    resume_condition(&semaphore->condition, &resume, TRUE);

    /* Clear the semaphore memory. */
    memset(semaphore, 0,  sizeof(SEMAPHORE));

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
    suspend->flags = (semaphore->type & SEMAPHORE_PRIORITY ? SUSPEND_PRIORITY : 0);
    suspend->do_suspend = &semaphore_do_suspend;
    suspend->timeout = timeout;

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
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Save the current task pointer. */
    tcb = get_current_task();

    /* If this is IRQ accessible semaphore. */
    if (semaphore->type & SEMAPHORE_IRQ)
    {
        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();
    }

    /* Lock the scheduler. */
    scheduler_lock();

    /* Should never happen. */
    OS_ASSERT((tcb) && ((tcb->irq_lock_count > 0) && (semaphore->type & SEMAPHORE_IRQ)));

    /* Check if this semaphore is not available. */
    if (semaphore->count == 0)
    {
        /* Should never happen. */
        OS_ASSERT((semaphore->type & SEMAPHORE_IRQ) && (semaphore->irq_lock == NULL));

        /* Check if we need to wait for semaphore to be free. */
        if ((wait > 0) && (tcb != NULL))
        {
            /* Initialize suspend condition for this semaphore. */
            semaphore_condition_get(semaphore, &condition, suspend_ptr, wait);

            /* While we cannot get the lock. */
            while ((status == SUCCESS) && (semaphore->count == 0))
            {
                /* Start waiting on this semaphore. */
                status = suspend_condition(&condition, &suspend_ptr, NULL, TRUE);
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
        /* Should never happen. */
        OS_ASSERT(semaphore->count == 0);

        /* Save the owner for this semaphore. */
        semaphore->owner = tcb;

        /* Decrease the semaphore count. */
        semaphore->count --;
    }

    /* If this is IRQ accessible semaphore. */
    if (semaphore->type & SEMAPHORE_IRQ)
    {
        /* If semaphore was successfully obtained. */
        if (status == SUCCESS)
        {
            /* Check if we have IRQ lock registered for this semaphore. */
            if (semaphore->irq_lock != NULL)
            {
                /* Lock the required interrupt. */
                semaphore->irq_lock(semaphore->irq_data);

                /* Restore old interrupt level. */
                SET_INTERRUPT_LEVEL(interrupt_level);

                /* Enable scheduling. */
                scheduler_unlock();
            }
            else
            {
                /* If we are in a task. */
                if (tcb != NULL)
                {
                    /* Should never happen. */
                    OS_ASSERT(tcb->irq_lock_count == SCHEDULER_MAX_IRQ_LOCK);

                    /* Increment the IRQ lock count. */
                    tcb->irq_lock_count++;
                }

                /* Save the IRQ interrupt level and scheduler state. */
                semaphore->irq_status = interrupt_level;
            }
        }
        else
        {
            /* Restore old interrupt level. */
            SET_INTERRUPT_LEVEL(interrupt_level);
        }
    }

    else
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

    /* If this is not IRQ accessible semaphore. */
    if ((!(semaphore->type & SEMAPHORE_IRQ)) || (semaphore->irq_unlock != NULL))
    {
        /* Lock the scheduler. */
        scheduler_lock();
    }

    /* Semaphore double release. */
    OS_ASSERT(semaphore->count >= semaphore->max_count);

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

    /* If this is IRQ accessible semaphore. */
    if (semaphore->type & SEMAPHORE_IRQ)
    {
        /* Check if we have IRQ unlock registered for this semaphore. */
        if (semaphore->irq_unlock != NULL)
        {
            /* Unlock the required interrupt. */
            semaphore->irq_unlock(semaphore->irq_data);
        }
        else
        {
            /* If we have a task. */
            if (tcb != NULL)
            {
                /* Should never happen. */
                OS_ASSERT(tcb->irq_lock_count == 0);

                /* Decrement the IRQ lock count. */
                tcb->irq_lock_count--;
            }

            /* If we are not suspending. */
            if ((tcb == NULL) || ((tcb->status != TASK_WILL_SUSPENDED) && (tcb->irq_lock_count == 0)))
            {
                /* Restore the IRQ interrupt level. */
                SET_INTERRUPT_LEVEL(semaphore->irq_status);
            }
        }
    }

    /* Enable scheduling. */
    scheduler_unlock();

} /* semaphore_release */

#endif /* CONFIG_SEMAPHORE */
