/*
 * condition.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */

#include <kernel.h>
#include <condition.h>
#include <sll.h>

#ifdef CONFIG_SLEEP
#include <sleep.h>
#endif

/* Internal function prototypes. */
static uint8_t suspend_sreach(void *, void *);
static void suspend_lock_condition(CONDITION **, SUSPEND **, uint8_t);
static void suspend_unlock_condition(CONDITION **, SUSPEND **, uint8_t);
static void suspend_condition_add_task(CONDITION **, SUSPEND **, uint8_t, TASK *);
static void suspend_condition_remove_all(CONDITION **, SUSPEND **, uint8_t);
static void suspend_condition_remove(CONDITION **, SUSPEND **, uint8_t, CONDITION *, uint8_t *);
static uint8_t suspend_do_suspend(CONDITION **, SUSPEND **, uint8_t, uint8_t *);
static uint8_t suspend_is_task_waiting(TASK *, CONDITION *);
#ifdef CONFIG_SLEEP
static uint32_t suspend_timeout_get_min(SUSPEND **, uint8_t, uint8_t *);
#endif

/*
 * suspend_sreach
 * @node: A task waiting on this condition.
 * @param: Resumption criteria.
 * @return: TRUE if we need to resume this task, FALSE if we cannot resume
 *  this task.
 * This is a search function to search a task that satisfy the suspension
 * criteria.
 */
static uint8_t suspend_sreach(void *node, void *param)
{
    SUSPEND *suspend = (SUSPEND *)node;
    RESUME *resume = (RESUME *)param;
    uint8_t match = TRUE;

    /* Check if we can resume this task. */
    match = resume->do_resume(resume->param, suspend->param);

    /* Return if we need to stop the search or need to process more. */
    return (match);

} /* suspend_sreach */

/*
 * suspend_unlock_condition
 * @condition: Condition list that we need to unlock.
 * @suspend: Suspend list for these conditions.
 * @num: Number of conditions.
 * This routine will unlock all the conditions in the condition list, except
 * the one for which we were resumed.
 */
static void suspend_unlock_condition(CONDITION **condition, SUSPEND **suspend, uint8_t num)
{
    /* Unlock all conditions. */
    while (num)
    {
        /* If we can unlock this condition. */
        if (((*suspend)->status >= SUCCESS) && ((*condition)->unlock))
        {
            /* Unlock this condition. */
            (*condition)->unlock((*condition)->data);
        }

        /* Pick next condition. */
        condition++;
        suspend++;
        num--;
    }

} /* suspend_unlock_condition */

/*
 * suspend_lock_condition
 * @condition: Condition list for which we need to get lock.
 * @suspend: Suspend list for these conditions.
 * @num: Number of conditions.
 * This routine will lock all the conditions.
 */
static void suspend_lock_condition(CONDITION **condition, SUSPEND **suspend, uint8_t num)
{
    /* For all conditions acquire lock for which we can suspend. */
    while (num)
    {
        /* If we can lock this condition. */
        if (((*suspend)->status >= SUCCESS) && ((*condition)->lock))
        {
            /* Get lock for this condition. */
            (*condition)->lock((*condition)->data);
        }

        /* Pick next condition. */
        condition++;
        suspend++;
        num--;
    }

} /* suspend_lock_condition */

/*
 * suspend_condition_add_task
 * @condition: Condition list in which we need to add this task.
 * @suspend: Suspend list.
 * @num: Number of conditions.
 * @tcb: Current task pointer.
 * This routine will add the given task on all the conditions we need to wait
 * for.
 */
static void suspend_condition_add_task(CONDITION **condition, SUSPEND **suspend, uint8_t num, TASK *tcb)
{
    /* For all conditions add this task. */
    while (num)
    {
        /* Add this task on the suspend data. */
        (*suspend)->task = tcb;

        /* Add suspend to the suspend list. */
        sll_append(&(*condition)->suspend_list, *suspend, OFFSETOF(SUSPEND, next));

        /* Pick next condition. */
        condition++;
        suspend++;
        num--;
    }

} /* suspend_condition_add_task */

/*
 * suspend_condition_remove_all
 * @condition: Condition list from which this suspend is needed to be removed.
 * @suspend: Suspend list.
 * @num: Number of conditions.
 * This routine will remove all the suspends from their respective conditions.
 */
static void suspend_condition_remove_all(CONDITION **condition, SUSPEND **suspend, uint8_t num)
{
    /* For all conditions remove respective conditions. */
    while (num)
    {
        /* Remove this suspend from the suspend list. */
        ASSERT(sll_remove(&(*condition)->suspend_list, *suspend, OFFSETOF(SUSPEND, next)) != *suspend);

        /* Pick next condition. */
        condition++;
        suspend++;
        num--;
    }

} /* suspend_condition_remove_all */

/*
 * suspend_condition_remove
 * @condition: Condition list from which this suspend is needed to be removed.
 * @suspend: Suspend list.
 * @resume_condition: Condition because of which we were resumed.
 * @num: Number of conditions.
 * @return_num: If not null the condition index for which we were resumed will
 *  be returned here.
 * This routine will remove all the suspends from their respective conditions,
 * except the one we were resumed from as it was already removed.
 */
static void suspend_condition_remove(CONDITION **condition, SUSPEND **suspend, uint8_t num, CONDITION *resume_condition, uint8_t *return_num)
{
    uint8_t n;
    uint8_t old_priority = SUSPEND_INVALID_PRIORITY;

    /* For all conditions remove this task. */
    for (n = 0; n < num; n++)
    {
        /* If this is the condition from which we got resumed. */
        if (resume_condition != (*condition))
        {
            /* We are no longer waiting on this condition remove this task from
             * the condition. */
            ASSERT(sll_remove(&(*condition)->suspend_list, *suspend, OFFSETOF(SUSPEND, next)) != *suspend);
        }

        /* If a condition has an error or we can resume from this suspend and
         * it has a higher priority. */
        if (((*suspend)->status < SUCCESS) || (((*suspend)->may_resume) && (old_priority > (*suspend)->priority)))
        {
            /* Return this condition index. */
            *return_num = n;
        }

        /* Pick next condition. */
        condition++;
        suspend++;
    }

} /* suspend_condition_remove */

/*
 * suspend_do_suspend
 * @condition: Condition list for which we need to check if we do need to
 *  suspend.
 * @suspend: Suspend list from which we need to check if we do need to suspend.
 * @num: Number of conditions.
 * @return_num: If a condition is valid, the index for that condition will be
 *  returned here.
 * @return: Will return true if we do need to suspend on a condition.
 * This routine will check for all the conditions to see if we do need to
 * suspend on them. If any of the condition is satisfied we will not suspend.
 */
static uint8_t suspend_do_suspend(CONDITION **condition, SUSPEND **suspend, uint8_t num, uint8_t *return_num)
{
    uint8_t n, do_suspend = TRUE;
    uint8_t old_priority = SUSPEND_INVALID_PRIORITY;

    /* For all conditions check if we need to suspend. */
    for (n = 0; n < num; n++)
    {
        /* Check if we don't need to suspend for this condition, or if user
         * has sent a ping on this condition. */
        if ((((*condition)->do_suspend) && ((*condition)->do_suspend((*condition)->data, (*suspend)->param) == FALSE)) || ((*condition)->flags & CONDITION_PING))
        {
            /* We don't need to suspend for this condition. */
            do_suspend = FALSE;

            /* If this suspend has the higher priority. */
            if (old_priority > (*suspend)->priority)
            {
                /* Return index for this condition. */
                *return_num = n;

                /* Update the old priority. */
                old_priority = (*suspend)->priority;
            }
        }
        else
        {
            /* Clear the may resume flag, as we might end up suspending for
             * this. */
            (*suspend)->may_resume = FALSE;
        }

        /* Pick next condition. */
        condition++;
        suspend++;
    }

    /* Return if we need to suspend. */
    return (do_suspend);

} /* suspend_do_suspend */

/*
 * suspend_is_task_waiting
 * @check_condition: Condition for which we need to check if this task is
 *  waiting on.
 * @return: TRUE if this task is waiting on the given condition,
 *  FALSE will be returned if this task is not waiting on the given
 *  condition.
 * This function will check and return if the given task is suspended on the
 * given condition.
 */
static uint8_t suspend_is_task_waiting(TASK *task, CONDITION *check_condition)
{
    uint8_t n, waiting = FALSE;

    /* Check the task list. */
    for (n = 0; n < task->num_conditions; n++)
    {
        /* If task is waiting on this condition. */
        if (((CONDITION **)task->suspend_data)[n] == check_condition)
        {
            /* We are waiting on this condition. */
            waiting = TRUE;

            break;
        }
    }

    /* Return if the task is waiting on given condition. */
    return (waiting);

} /* suspend_is_task_waiting */

#ifdef CONFIG_SLEEP
/*
 * suspend_timeout_get_min
 * @suspend: Suspend list from which we will search for minimum timeout.
 * @num: Number of suspends.
 * @return_num: The index at which first minimum timeout was found will be
 *  returned here.
 * @return: Minimum timeout calculated will be returned here.
 * This routine will calculate the timeout for which we need to wait on the
 * given conditions before returning an error.
 */
static uint32_t suspend_timeout_get_min(SUSPEND **suspend, uint8_t num, uint8_t *return_num)
{
    uint32_t min_timeout = MAX_WAIT, this_timeout, clock = current_system_tick();
    uint8_t n, min_index = 0;

    /* For all conditions search the minimum timeout. */
    for (n = 0; n < num; n++)
    {
        /* If timer is enabled for this suspend condition. */
        if ((*suspend)->timeout_enabled == TRUE)
        {
            /* Calculate the number of ticks left till it's timeout. */
            this_timeout = (INT32CMP((*suspend)->timeout, clock) > 0) ? (uint32_t)INT32CMP((*suspend)->timeout, clock) : 0;

            /* If this timer has minimum ticks left on it. */
            if (this_timeout < min_timeout)
            {
                /* Update the minimum timeout. */
                min_timeout = this_timeout;

                /* Save the entry index. */
                min_index = n;
            }
        }

        /* Pick next condition. */
        suspend++;
    }

    /* Return the condition index. */
    *return_num = min_index;

    /* Return the calculated minimum timeout. */
    return (min_timeout);

} /* suspend_timeout_get_min */
#endif /* CONFIG_SLEEP */

/*
 * suspend_condition
 * @condition: Array of conditions for which we need to suspend this task.
 * @suspend: Array of suspend data associated with each condition.
 * @num: Pointer to number of conditions we are waiting for. Can be null for
 *  one condition otherwise the index of the condition for which we resumed
 *  will be returned here.
 * @locked: If TRUE the caller is already locked, and we will return in the
 *  locked state. If FALSE the caller is not locked and we will return in non
 *  locked state.
 * @return: SUCCESS if criteria was successfully achieved,
 *  CONDITION_TIMEOUT will be returned if a timeout was occurred while waiting
 *  for the condition.
 * This function will suspend the caller task to wait for a criteria.
 */
int32_t suspend_condition(CONDITION **condition, SUSPEND **suspend, uint8_t *num, uint8_t locked)
{
    uint32_t timeout;
    int32_t status = SUCCESS;
    CONDITION *resume_condition = NULL;
    TASK *tcb = get_current_task();
    INT_LVL interrupt_level;
    uint8_t timeout_index, num_conditions, return_num, task_state = TASK_RESUME;

    /* Current task should not be null. */
    ASSERT(tcb == NULL);

#ifndef CONFIG_SLEEP
    /* Remove some compiler warning. */
    UNUSED_PARAM(timeout);
#endif

    /* If more than one condition was given. */
    if (num != NULL)
    {
        /* Pick the number of conditions given. */
        return_num = num_conditions = *num;
    }
    else
    {
        /* We have only one condition to process. */
        return_num = num_conditions = 1;
    }

    /* If caller is not in locked state. */
    if (locked == FALSE)
    {
        /* Lock all conditions before using them. */
        suspend_lock_condition(condition, suspend, num_conditions);
    }

    /* Calculate the minimum timeout we need to wait for the conditions. */
    timeout = suspend_timeout_get_min(suspend, num_conditions, &timeout_index);

    /* There is never a surety that if a condition is satisfied for a task when
     * it is resumed, as some other high priority task may again trigger in
     * and avail that condition. */
    /* This is not a bug and happen in a RTOS where different types of
     * schedulers are present. */

    /* Check if we need to suspend on this condition. */
    while (suspend_do_suspend(condition, suspend, num_conditions, &return_num))
    {
        /* Add this task on all the conditions. */
        suspend_condition_add_task(condition, suspend, num_conditions, tcb);

        /* Disable preemption. */
        scheduler_lock();

#ifdef CONFIG_SLEEP
        /* Check if we need to wait for a finite time. */
        if (timeout != MAX_WAIT)
        {
            /* Add the current task to the sleep list, if not available in
             * the allowed time the task will be resumed. */
            sleep_add_to_list(tcb, timeout);
        }
#endif /* CONFIG_SLEEP */

        /* Disable global interrupts, need to do this to protect against any
         * interrupt locks. */
        interrupt_level = GET_INTERRUPT_LEVEL();
        DISABLE_INTERRUPTS();

        /* Unlock all the conditions so they can be resumed. */
        suspend_unlock_condition(condition, suspend, num_conditions);

        /* Task is being suspended. */
        tcb->state = TASK_SUSPENDED;

        /* Assign the suspension data to the task. */
        tcb->num_conditions = num_conditions;
        tcb->suspend_data = (void *)condition;

        /* Wait for either being resumed by some data or timeout. */
        CONTROL_TO_SYSTEM();

        /* Save task state and the condition from which we are resumed. */
        /* It is assumed that the resume condition is also present in the
         * condition list, otherwise it will cause invalid behavior. */
        task_state = tcb->state;
        resume_condition = tcb->suspend_data;

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* Enable preemption. */
        scheduler_unlock();

        /* Lock all conditions except the one which caused an error. */
        suspend_lock_condition(condition, suspend, num_conditions);

        /* Check if we are resumed due to a timeout. */
        if (task_state == TASK_SLEEP_RESUME)
        {
            /* Remove this task from all the conditions. */
            suspend_condition_remove_all(condition, suspend, num_conditions);

            /* Return an error we failed to achieve the condition. */
            status = CONDITION_TIMEOUT;

            /* Return the index of the timed out condition. */
            return_num = timeout_index;

            /* Break out of the loop. */
            break;
        }

        else
        {
#ifdef CONFIG_SLEEP
            /* Check if we were waiting on a timeout. */
            if (timeout != MAX_WAIT)
            {
                /* Disable preemption. */
                scheduler_lock();

                /* Remove this task from sleeping tasks. */
                sleep_remove_from_list(tcb);

                /* Enable preemption. */
                scheduler_unlock();
            }
#endif /* CONFIG_SLEEP */

            /* Remove the task from all the conditions except the one from
             * which we resumed. */
            suspend_condition_remove(condition, suspend, num_conditions, resume_condition, &return_num);

            /* If we did not resume normally. */
            if (suspend[return_num]->status != SUCCESS)
            {
                /* Return the status provided by resume. */
                status = suspend[return_num]->status;

                /* Break out of the loop. */
                break;
            }

            /* If a ping resumed this condition. */
            if (condition[return_num]->flags & CONDITION_PING)
            {
                /* Break out of the loop. */
                break;
            }

            /* Check if we still don't need to suspend for the condition we
             * resumed from. */
            if ((condition[return_num]->do_suspend) && (condition[return_num]->do_suspend(condition[return_num]->data, suspend[return_num]->param) == FALSE))
            {
                /* Break out of the loop. */
                break;
            }
        }
    }

    /* If a ping resumed this condition. */
    if (condition[return_num]->flags & CONDITION_PING)
    {
        /* Clear the ping flag. */
        condition[return_num]->flags &= (uint8_t)~(CONDITION_PING);
    }

    /* If caller was not in locked state. */
    if (locked == FALSE)
    {
        /* Unlock all conditions, if we did not resume normally or timeout
         * don't unlock the one for which we resumed as we did not lock it.  */
        suspend_unlock_condition(condition, suspend, num_conditions);
    }

    /* If we need to return the condition from which we resumed. */
    if (*num != NULL)
    {
        /* Return the required condition. */
        *num = return_num;
    }

    /* Return status to the caller. */
    return (status);

} /* suspend_condition */

/*
 * resume_condition
 * @condition: Condition for which we need to resume tasks.
 * @resume: Resume data.
 * @locked: If we are resuming task(s) on a condition in locked state.
 * This will resume any tasks waiting for a condition.
 */
void resume_condition(CONDITION *condition, RESUME *resume, uint8_t locked)
{
    SUSPEND *suspend;
    SUSPEND_LIST tmp_list = {NULL, NULL};
    INT_LVL interrupt_level;
    uint8_t yield_task = FALSE;

    /* If caller is not in locked state. */
    if ((locked == FALSE) && (condition->lock))
    {
        /* Lock this condition. */
        condition->lock(condition->data);
    }

    /* Disable preemption as we will be accessing task data structure. */
    scheduler_lock();

    /* Resume all the tasks waiting on this condition. */
    do
    {
        /* If a parameter was given. */
        if ((resume != NULL) && (resume->param != NULL))
        {
            /* Should never happen. */
            ASSERT(resume->do_resume == NULL);

            /* Search for a task that can be resumed. */
            suspend = (SUSPEND *)sll_search_pop(&condition->suspend_list, &suspend_sreach, resume, OFFSETOF(SUSPEND, next));
        }

        else
        {
            /* Get a task that can be executed. */
            suspend = (SUSPEND *)sll_pop(&condition->suspend_list, OFFSETOF(SUSPEND, next));
        }

        /* If we have a task. */
        if (suspend)
        {
            /* Disable interrupts to protect access to resources from interrupts. */
            interrupt_level = GET_INTERRUPT_LEVEL();
            DISABLE_INTERRUPTS();

            /* If we do have resume data. */
            if (resume != NULL)
            {
                /* Return the given status. */
                suspend->status = resume->status;
            }
            else
            {
                /* Just return the success. */
                suspend->status = SUCCESS;
            }

            /* Mark as this suspend may resume. */
            suspend->may_resume = TRUE;

            /* If task is actually suspended on this condition. */
            if ((suspend->task->state == TASK_SUSPENDED) && (suspend_is_task_waiting(suspend->task, condition) == TRUE))
            {
                /* Save the condition for which this task is being resumed. */
                suspend->task->suspend_data = condition;

                /* Try to reschedule this task. */
                scheduler_task_yield(suspend->task, YIELD_SYSTEM);

                /* Yield the current task. */
                yield_task = TRUE;
            }
            else
            {
                /* Save this task in our temporary list, we will put it back on
                 * the list later. */
                sll_push(&tmp_list, suspend, OFFSETOF(SUSPEND, next));
            }

            /* Restore old interrupt level. */
            SET_INTERRUPT_LEVEL(interrupt_level);
        }

    } while (suspend != NULL);

    /* Enable preemption, we will switch to a new task here if
     * required. */
    scheduler_unlock();

    /* If we need to yield the current task. */
    if (yield_task == TRUE)
    {
        /* Try to yield the current task. */
        task_yield();
    }

    /* Put any tasks back on the suspend list if any. */
    do
    {
        /* Get a task we need to put back on the suspend list. */
        suspend = (SUSPEND *)sll_pop(&tmp_list, OFFSETOF(SUSPEND, next));

        /* If we do have a task. */
        if (suspend != NULL)
        {
            /* Push this task back on the suspend list we will remove it when
             * we will resume. */
            sll_push(&condition->suspend_list, suspend, OFFSETOF(SUSPEND, next));
        }

    } while (suspend != NULL);

    /* If caller was not in locked state. */
    if ((locked == FALSE) && (condition->unlock))
    {
        /* Unlock this condition. */
        condition->unlock(condition->data);
    }

} /* resume_condition */
