/*
 * condition.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */

#include <os.h>
#include <condition.h>
#include <sll.h>

#ifdef CONFIG_SLEEP
#include <sleep.h>
#endif

/* Internal function prototypes. */
static uint8_t suspend_sreach(void *, void *);
static uint8_t suspend_priority_sort(void *, void *);
static void suspend_lock_condition(CONDITION **, uint32_t, CONDITION *);
static void suspend_unlock_condition(CONDITION **, uint32_t, CONDITION *);
static void suspend_condition_add_task(CONDITION **, SUSPEND **, uint32_t, TASK *);
static void suspend_condition_remove_all(CONDITION **, SUSPEND **, uint32_t);
static void suspend_condition_remove(CONDITION **, SUSPEND **, uint32_t, CONDITION *, uint32_t *);
static uint8_t suspend_do_suspend(CONDITION **, SUSPEND **, uint32_t, uint32_t *);
static uint32_t suspend_timeout_get_min(SUSPEND **, uint32_t, uint32_t *);
#ifdef CONFIG_SLEEP
static void suspend_condition_adjust_timers(SUSPEND **, uint32_t, uint32_t);
#endif

/*
 * suspend_sreach
 * @node: An task waiting on this condition.
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
 * suspend_priority_sort
 * @node: Existing suspend in a list.
 * @task: New suspend being added in a list.
 * @return: TRUE if the new suspend is needed be handled before the existing
 *  suspend in the list.
 * This is sorting function called by SLL routines to sort the suspend list on
 * the priority.
 */
static uint8_t suspend_priority_sort(void *node, void *new)
{
    uint8_t schedule = FALSE;
    SUSPEND *suspend_node = (SUSPEND *)node, *suspned_new = (SUSPEND *)new;

    /* Check if this node has lower priority than the new suspend. */
    if (suspend_node->task->priority > suspned_new->task->priority)
    {
        /* Handle the new suspend before this one. */
        schedule = TRUE;
    }

    /* Return if we need to handle this suspend before the given suspend. */
    return (schedule);

} /* suspend_priority_sort. */

/*
 * suspend_unlock_condition
 * @condition: Condition list that we need to unlock.
 * @num: Number of conditions.
 * @resume_condition: Condition for which we don't need to acquire lock.
 * This routine will unlock all the conditions in the condition list, except
 * the one that was returned on the resume.
 */
static void suspend_unlock_condition(CONDITION **condition, uint32_t num, CONDITION *resume_condition)
{
    /* Unlock all conditions. */
    while (num)
    {
        /* If we can unlock this condition. */
        if (((!resume_condition) || (resume_condition != (*condition))) && ((*condition)->unlock))
        {
            /* Unlock this condition. */
            (*condition)->unlock((*condition)->data);
        }

        /* Pick next condition. */
        condition++;

        /* This is now processed. */
        num--;
    }

} /* suspend_unlock_condition */

/*
 * suspend_lock_condition
 * @condition: Condition list for which we need to get lock.
 * @num: Number of conditions.
 * @tcb: Condition for which we don't need to acquire lock.
 * This routine will lock the conditions.
 */
static void suspend_lock_condition(CONDITION **condition, uint32_t num, CONDITION *resume_condition)
{
    /* For all conditions do post. */
    while (num)
    {
        /* If we can lock this condition. */
        if (((!resume_condition) || (resume_condition != (*condition))) && ((*condition)->lock))
        {
            /* Get lock for this condition. */
            (*condition)->lock((*condition)->data);
        }

        /* Pick next condition. */
        condition++;

        /* This is now processed. */
        num--;
    }

} /* suspend_lock_condition */

/*
 * suspend_condition_add_task
 * @condition: Condition list in which we need to add this task.
 * @suspend: Suspend list.
 * @num: Number of conditions.
 * @tcb: Current task pointer.
 * This routine will add given task on all the conditions we need to wait for.
 */
static void suspend_condition_add_task(CONDITION **condition, SUSPEND **suspend, uint32_t num, TASK *tcb)
{
    /* For all conditions add this task. */
    while (num)
    {
        /* Add this task on the suspend data. */
        (*suspend)->task = tcb;

        /* If we need to sort the list on priority. */
        if ((*suspend)->flags & CONDITION_PRIORITY)
        {
            /* Add suspend to the suspend list on priority. */
            sll_insert(&(*condition)->suspend_list, *suspend, &suspend_priority_sort, OFFSETOF(SUSPEND, next));
        }

        else
        {
            /* Add suspend at the end of suspend list. */
            sll_append(&(*condition)->suspend_list, *suspend, OFFSETOF(SUSPEND, next));
        }

        /* Pick next condition. */
        condition++;
        suspend++;

        /* This is now processed. */
        num--;
    }

} /* suspend_condition_add_task */

/*
 * suspend_condition_remove_all
 * @condition: Condition list from which this suspend is needed to be removed.
 * @suspend: Suspend list.
 * @num: Number of conditions.
 * This routine will remove all the suspend from the respective conditions.
 */
static void suspend_condition_remove_all(CONDITION **condition, SUSPEND **suspend, uint32_t num)
{
    /* For all conditions remove respective conditions. */
    while (num > 0)
    {
        /* Remove this suspend from the suspend list. */
        OS_ASSERT(sll_remove(&(*condition)->suspend_list, *suspend, OFFSETOF(SUSPEND, next)) != *suspend);

        /* Pick next condition. */
        condition++;
        suspend++;

        /* This is now processed. */
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
 * This routine will remove all the suspend from the respective conditions,
 * except the one we were resumed from at it was already removed.
 */
static void suspend_condition_remove(CONDITION **condition, SUSPEND **suspend, uint32_t num, CONDITION *resume_condition, uint32_t *return_num)
{
    uint32_t n;

    /* For all conditions remove this task. */
    for (n = 0; n < num; n++)
    {
        /* If this is the condition from which we got resumed. */
        if (resume_condition == (*condition))
        {
            /* Return the condition index that was matched. */
            *return_num = n;
        }
        else
        {
            /* We are no longer waiting on this condition remove this task from
             * the condition. */
            OS_ASSERT(sll_remove(&(*condition)->suspend_list, *suspend, OFFSETOF(SUSPEND, next)) != *suspend);
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
 * @return_num: If a condition is valid the index for that condition will be
 *  returned here.
 * @return: Will return true if we do need to suspend on a condition.
 * This routine will check for all the conditions if we do need to suspend on
 * them. If any of the condition is valid we will not suspend to wait for that
 * condition.
 */
static uint8_t suspend_do_suspend(CONDITION **condition, SUSPEND **suspend, uint32_t num, uint32_t *return_num)
{
    uint8_t do_suspend = TRUE;
    uint32_t n;

    /* For all conditions check if we need to suspend. */
    for (n = 0; n < num; n++)
    {
        /* Check if we don't need to suspend for this condition. */
        if (((*suspend)->do_suspend) && ((*suspend)->do_suspend((*condition)->data, (*suspend)->param) == FALSE))
        {
            /* We don't need to suspend for this condition. */
            do_suspend = FALSE;

            /* Return index for this condition. */
            *return_num = n;

            /* Break out of this loop. */
            break;
        }

        /* Pick next condition. */
        condition++;
        suspend++;
    }

    /* Return if we need to suspend. */
    return (do_suspend);

} /* suspend_do_suspend */

/*
 * suspend_timeout_get_min
 * @condition: Condition list for which we will calculate the the minimum
 *  timeout we need to wait.
 * @suspend: Suspend list from which we will search for timeout for
 *  corresponding condition.
 * @num: Number of conditions.
 * @return_num: The index at which first minimum timeout was found will be
 *  returned here.
 * @return: Minimum timeout calculated will be returned here.
 * This routine will calculate the minimum number of times we need to wait on
 * the given conditions before returning a timeout.
 */
static uint32_t suspend_timeout_get_min(SUSPEND **suspend, uint32_t num, uint32_t *return_num)
{
    uint32_t n, min_timeout = (*suspend)->timeout, min_index = 0;

    /* Pick next condition. */
    suspend++;

    /* For all conditions search the minimum timeout. */
    for (n = 1; n < num; n++)
    {
        /* Check if we don't need to suspend for this condition. */
        if ((*suspend)->timeout < min_timeout)
        {
            /* Update the minimum timeout. */
            min_timeout = (*suspend)->timeout;

            /* Save the entry index. */
            min_index = n;
        }

        /* Pick next condition. */
        suspend++;
    }

    /* Return the condition index. */
    *return_num = min_index;

    /* Return the calculated minimum timeout. */
    return (min_timeout);

} /* suspend_timeout_get_min */

#ifdef CONFIG_SLEEP
/*
 * suspend_condition_adjust_timers
 * @suspend: Suspend list for which timers are needed to be updated.
 * @num: Number of conditions.
 * @ticks: Number of ticks passed since we waited for this condition.
 * This routine will adjust timeouts for all the timers in the condition list.
 */
static void suspend_condition_adjust_timers(SUSPEND **suspend, uint32_t num, uint32_t ticks)
{
    /* For all conditions find the timer conditions. */
    while (num > 0)
    {
        /* If this was a timer suspend and we did not want to suspend indefinitely. */
        if (((*suspend)->flags & CONDITION_TIMER) && ((*suspend)->timeout != MAX_WAIT))
        {
            /* Adjust the timeout for this suspend. */
            (*suspend)->timeout = (*suspend)->timeout - (((*suspend)->timeout > ticks) ? ticks : (*suspend)->timeout);
        }

        /* Pick next condition. */
        suspend++;

        /* This is now processed. */
        num--;
    }

} /* suspend_condition_adjust_timers */
#endif /* CONFIG_SLEEP */

/*
 * suspend_condition
 * @condition: Condition for which we need to suspend this task.
 * @suspend: Suspend data.
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
int32_t suspend_condition(CONDITION **condition, SUSPEND **suspend, uint32_t *num, uint8_t locked)
{
#ifdef CONFIG_SLEEP
    uint64_t last_tick = current_system_tick();
#endif
    TASK *tcb = get_current_task();
    int32_t status = SUCCESS, task_status = TASK_RESUME;
    uint32_t num_conditions = *num, timeout, timeout_index;
    CONDITION *resume_condition;

#ifndef CONFIG_SLEEP
    /* Remove some compiler warning. */
    UNUSED_PARAM(timeout);
#endif

    /* Current task should not be null. */
    OS_ASSERT(tcb == NULL);

    /* Disable preemption. */
    scheduler_lock();

    /* If more than one condition was given. */
    if (num != NULL)
    {
        /* Pick the number of conditions given. */
        num_conditions = *num;
    }
    else
    {
        /* We have only one condition to process. */
        num_conditions = 1;
    }

    /* If caller is not in locked state. */
    if (locked == FALSE)
    {
        /* Lock all conditions before using them. */
        suspend_lock_condition(condition, num_conditions, NULL);
    }

    /* Calculate the minimum timeout we need to wait for the conditions. */
    timeout = suspend_timeout_get_min(suspend, num_conditions, &timeout_index);

    /* There is never a surety that if a condition is satisfied for a task when
     * it is resumed, as some other high priority task may again trigger in
     * and avail that condition. */
    /* This is not a bug and happen in a RTOS where different types of
     * schedulers are present. */

    /* Check if we need to suspend on this condition. */
    while (suspend_do_suspend(condition, suspend, num_conditions, num))
    {
#ifdef CONFIG_SLEEP
        /* Check if we need to wait for a finite time. */
        if (timeout != (uint32_t)(MAX_WAIT))
        {
            /* Use the last tick to save the number of ticks we want to skim
             * from the timeout. */
            last_tick -= current_system_tick();

            /* If we do need to update the timeouts. */
            if (last_tick > 0)
            {
                /* Update the timer conditions reflect the number of ticks remaining on
                 * their timeouts. */
                suspend_condition_adjust_timers(suspend, num_conditions, (uint32_t)last_tick);

                /* Remove these from the actual number of ticks we will be
                 * waiting on this condition. */
                timeout -= (uint32_t)last_tick;
            }

            /* Save when we suspended last time. */
            last_tick = current_system_tick();

            /* Add the current task to the sleep list, if not available in
             * the allowed time the task will be resumed. */
            sleep_add_to_list(tcb, timeout);
        }
#endif /* CONFIG_SLEEP */

        /* Add this task on all the conditions. */
        suspend_condition_add_task(condition, suspend, num_conditions, tcb);

        /* Task is being suspended. */
        tcb->status = TASK_SUSPENDED;

        /* Assign the suspension data to the task. */
        tcb->suspend_data = (void *)suspend;

        /* Unlock all the conditions so they can be resumed. */
        suspend_unlock_condition(condition, num_conditions, NULL);

        /* Wait for either being resumed by some data or timeout. */
        task_waiting();

        /* Save task status and the condition from which we are resumed. */
        task_status = tcb->status;
        resume_condition = tcb->suspend_data;

        /* If we resumed normally or got timed out. */
        if ((task_status == TASK_RESUME_SLEEP) || (task_status == TASK_RESUME))
        {
            /* Lock all the conditions. */
            suspend_lock_condition(condition, num_conditions, NULL);
        }
        else
        {
            /* Lock all the conditions except the one from which we are
             * resuming. */
            suspend_lock_condition(condition, num_conditions, resume_condition);
        }

        /* Check if we are resumed due to a timeout. */
        if (task_status == TASK_RESUME_SLEEP)
        {
            /* Remove this task from all the conditions. */
            suspend_condition_remove_all(condition, suspend, num_conditions);

            /* Return an error we failed to achieve the condition. */
            status = CONDITION_TIMEOUT;

            /* Return the index of the timed out condition. */
            *num = timeout_index;

            /* Break out of the loop. */
            break;
        }

        else
        {
            /* Remove the task from all the conditions except the one from
             * which we resumed. */
            suspend_condition_remove(condition, suspend, num_conditions, resume_condition, num);

            /* If we did not resume normally. */
            if (task_status != TASK_RESUME)
            {
                /* Return the error returned by the task. */
                status = task_status;

                /* Break out of the loop. */
                break;
            }
        }
    }

    /* If caller was not in locked state. */
    if (locked == FALSE)
    {
        /* Unlock all conditions, if we did not resume normally or timeout
         * don't unlock the one for which we resumed as we did not lock it.  */
        suspend_unlock_condition(condition, num_conditions, ((task_status != TASK_RESUME) && (task_status != TASK_RESUME_SLEEP)) ? resume_condition : NULL);
    }

    /* Enable preemption. */
    scheduler_unlock();

    /* Return status to the caller. */
    return (status);

} /* suspend_condition */

/*
 * resume_condition
 * @condition: Condition for which we need to resume tasks.
 * @resume: Resume data.
 * @locked: If we are resuming task(s) on a condition in locked state.
 * @return: SUCCESS if criteria was successfully achieved,
 *  SUSPEND_PRIORITY will be returned if a timeout was occurred while waiting
 *  for the condition.
 * This function will suspend the caller task to wait for a criteria.
 */
void resume_condition(CONDITION *condition, RESUME *resume, uint8_t locked)
{
    SUSPEND *suspend;

    /* If caller is not in locked state. */
    if ((locked == FALSE) && (condition->lock))
    {
        /* Lock this condition. */
        condition->lock(condition->data);
    }

    /* Resume all the tasks waiting on this condition. */
    do
    {
        /* If a parameter was given. */
        if (resume->param != NULL)
        {
            /* Should never happen. */
            OS_ASSERT(resume->do_resume == NULL);

            /* Search for a task that can be resumed. */
            suspend = (SUSPEND *)sll_search_pop(&condition->suspend_list, &suspend_sreach, resume, OFFSETOF(SUSPEND, next));
        }

        else
        {
            /* Get the first task that can be executed. */
            suspend = (SUSPEND *)sll_pop(&condition->suspend_list, OFFSETOF(SUSPEND, next));
        }

        /* If we have a task, and we need to resume it. */
        if ((suspend) && (suspend->task->status == TASK_SUSPENDED))
        {
            /* Set the task status as required by resume. */
            suspend->task->status = resume->status;

            /* Save the condition for which this task is resuming. */
            suspend->task->suspend_data = condition;

#ifdef CONFIG_SLEEP
            /* Remove this task from sleeping tasks. */
            sleep_remove_from_list(suspend->task);
#endif /* CONFIG_SLEEP */

            /* Try to reschedule this task. */
            ((SCHEDULER *)(suspend->task->scheduler))->yield(suspend->task, YIELD_SYSTEM);

            /* Try to yield the current task. */
            task_yield();
        }

    } while (suspend != NULL);

    /* If caller was not in locked state. */
    if ((locked == FALSE) && (condition->unlock))
    {
        /* Unlock this condition. */
        condition->unlock(condition->data);
    }

} /* resume_condition */
