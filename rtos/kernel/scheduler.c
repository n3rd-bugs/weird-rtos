/*
 * scheduler.c
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
#include <scheduler.h>
#include <sll.h>
#include <sleep.h>
#include <string.h>
#include <idle.h>

/* A list of all the tasks in the system. */
#ifdef TASK_STATS
TASK_LIST sch_task_list;
#endif
TASK_LIST sch_ready_task_list;

/* Internal function prototypes. */
static uint8_t scheduler_task_sort(void *, void *);

/*
 * scheduler_init
 * This function initializes scheduler, this must be called before adding any
 * tasks in the system.
 */
void scheduler_init(void)
{
    /* Clear the schedule lists. */
#ifdef TASK_STATS
    memset(&sch_task_list, 0, sizeof(TASK_LIST));
#endif
    memset(&sch_ready_task_list, 0, sizeof(TASK_LIST));

    /* Initialize idle task. */
    idle_task_init();

} /* scheduler_init */

/*
 * scheduler_task_add
 * @tcb: Task control block that is needed to be added in the system.
 * @priority: Priority for this task.
 * This function adds a task in the system, the task must be initialized before
 * adding.
 */
void scheduler_task_add(TASK *tcb, uint8_t priority)
{
    INT_LVL interrupt_level;

    /* Disable interrupts. */
    interrupt_level = GET_INTERRUPT_LEVEL();
    DISABLE_INTERRUPTS();

    /* Update the task control block. */
    tcb->priority = priority;

#ifdef ASSERT_ENABLE
    /* If this is not the idle task. */
    if (tcb != &idle_task)
    {
        /* Validate priority level. */
        ASSERT((priority > SCHEDULER_MAX_PRI));
    }
#endif /* ASSERT_ENABLE */

    /* Enqueue this task in the ready list. */
    scheduler_task_yield(tcb, YIELD_SYSTEM);

#ifdef TASK_STATS
    /* Append this task to the global task list. */
    sll_append(&sch_task_list, tcb, OFFSETOF(TASK, next_global));
#endif /* TASK_STATS */

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

} /* scheduler_task_add */

/*
 * scheduler_task_remove
 * @tcb: Task control block that is needed to be removed.
 * This function removes a finished task from the global task list. Once
 * removed user can call scheduler_task_add to run a finished task.
 */
void scheduler_task_remove(TASK *tcb)
{
    /* Lock the scheduler. */
    scheduler_lock();

    /* Task should be in finished state. */
    ASSERT(tcb->state != TASK_FINISHED);

#ifdef TASK_STATS
    /* Remove this task from global task list. */
    sll_remove(&sch_task_list, tcb, OFFSETOF(TASK, next_global));
#endif /* TASK_STATS */

    /* Enable scheduling. */
    scheduler_unlock();

} /* scheduler_task_remove */

/*
 * scheduler_lock
 * This function will disable preemption for this task so that it cannot be
 * preempted.
 */
void scheduler_lock(void)
{
    TASK *tcb = get_current_task();

    /* Check if we have a current task. */
    if (tcb != NULL)
    {
        /* Should never happen. */
        ASSERT(tcb->lock_count >= SCHEDULER_MAX_LOCK);

        /* Increment the lock count for this task. */
        tcb->lock_count ++;
    }

} /* scheduler_lock */

/*
 * scheduler_unlock
 * This function will enable preemption for this task so that it can be
 * preempted.
 */
void scheduler_unlock(void)
{
    TASK *tcb = get_current_task();

    /* Check if we have a current task. */
    if (tcb != NULL)
    {
        /* Should never happen. */
        ASSERT(tcb->lock_count == 0);

        /* Decrement the lock count for this task. */
        tcb->lock_count --;

        /* If scheduler is actually unlocked, and we might have missed a
         * context switch because of this lock. */
        if ((tcb->lock_count == 0) && (tcb->flags & TASK_SCHED_DRIFT))
        {
            /* Clear the drift flag. */
            tcb->flags &= (uint8_t)(~(TASK_SCHED_DRIFT));

            /* Try to yield this task. */
            task_yield();
        }
    }

} /* scheduler_unlock */

/*
 * scheduler_get_next_task
 * @return: Task's control block that will run next.
 * This function is called by system tick to get the next task that is needed to
 * run next.
 */
TASK *scheduler_get_next_task(void)
{
    TASK *tcb = NULL;

#ifdef CONFIG_SLEEP
    /* Resume any of the sleeping tasks. */
    sleep_process_system_tick();
#endif /* CONFIG_SLEEP */

    /* Get the task we need to run */
    tcb = (TASK *)sll_pop(&sch_ready_task_list, OFFSETOF(TASK, next));

    /* We should always have a task to execute. */
    ASSERT(tcb == NULL);

#ifdef TASK_STATS
    /* Increment the number of times this task was scheduled. */
    tcb->scheduled ++;
#endif /* TASK_STATS */

    /* Return the task to run. */
    return (tcb);

} /* scheduler_get_next_task */

/*
 * scheduler_task_yield
 * @tcb: The task's control block that is needed to be scheduled in the
 *  aperiodic scheduler.
 * @from: From where this task is being scheduled.
 * This is yield function required by a scheduling class, this is called when a
 * task is needed to be scheduled in the aperiodic scheduler.
 */
void scheduler_task_yield(TASK *tcb, uint8_t from)
{
    /* Adjust the task control block as required. */
    switch (from)
    {
#ifdef CONFIG_SLEEP
    case YIELD_SLEEP:

        /* Task is being resumed from sleep. */
        tcb->tick_sleep = 0;
        tcb->state = TASK_SLEEP_RESUME;
        tcb->resume_from = TASK_RESUME_SLEEP;

        break;
#endif /* CONFIG_SLEEP */

    default:

        /* Task is resuming normally. */
        tcb->state = TASK_RESUME;
        tcb->resume_from = TASK_RESUME_SYSTEM;

        break;
    }

    /* Schedule the task being yielded/re-enqueued. */
    sll_insert(&sch_ready_task_list, tcb, &scheduler_task_sort, OFFSETOF(TASK, next));

} /* scheduler_task_yield */

/*
 * scheduler_task_sort
 * @node: An existing node in the list.
 * @task: New task that is needed to be added in the list.
 * @return: If the new task is needed to be scheduled before the existing node
 *  TRUE will be returned otherwise FALSE will be returned.
 * This is task sorting function that is used by SLL routines to schedule new
 * aperiodic tasks.
 */
static uint8_t scheduler_task_sort(void *node, void *task)
{
    uint8_t schedule = FALSE;

    /* If node has lower priority than the given task, then we can schedule the
     * given task here. */
    if (((TASK *)node)->priority > ((TASK *)task)->priority)
    {
        /* Schedule the given task before this node. */
        schedule = TRUE;
    }

    /* Return if we need to schedule this task before the given node. */
    return (schedule);

} /* scheduler_task_sort */
