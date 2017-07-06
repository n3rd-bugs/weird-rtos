/*
 * scheduler.c
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <scheduler.h>
#include <sll.h>
#include <sleep.h>
#include <string.h>
#include <idle.h>

/* A list of all the tasks in the system. */
#ifdef CONFIG_TASK_STATS
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
void scheduler_init()
{
    /* Clear the schedule lists. */
#ifdef CONFIG_TASK_STATS
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

    /* Enqueue this task in the ready list. */
    scheduler_task_yield(tcb, YIELD_INIT);

#ifdef CONFIG_TASK_STATS
    /* Append this task to the global task list. */
    sll_append(&sch_task_list, tcb, OFFSETOF(TASK, next_global));
#endif /* CONFIG_TASK_STATS */

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
    ASSERT(tcb->status != TASK_FINISHED);

#ifdef CONFIG_TASK_STATS
    /* Remove this task from global task list. */
    sll_remove(&sch_task_list, tcb, OFFSETOF(TASK, next_global));
#endif /* CONFIG_TASK_STATS */

    /* Enable scheduling. */
    scheduler_unlock();

} /* scheduler_task_remove */

/*
 * scheduler_lock
 * This function will disable preemption for this task so that it cannot be
 * preempted.
 */
void scheduler_lock()
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
void scheduler_unlock()
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
TASK *scheduler_get_next_task()
{
    TASK *tcb = NULL;

    /* Resume any of the sleeping tasks. */
    sleep_process_system_tick();

    /* Get the task we need to run */
    tcb = (TASK *)sll_pop(&sch_ready_task_list, OFFSETOF(TASK, next));

    /* We should always have a task to execute. */
    ASSERT(tcb == NULL);

#ifdef CONFIG_TASK_STATS
    /* Increment the number of times this task was scheduled. */
    tcb->scheduled ++;
#endif /* CONFIG_TASK_STATS */

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
    case YIELD_INIT:
    case YIELD_MANUAL:

        /* Task is resuming normally. */
        tcb->status = TASK_RESUME;

        break;

    case YIELD_SLEEP:

        /* Task is being resumed from sleep. */
        tcb->tick_sleep = 0;
        tcb->status = TASK_RESUME_SLEEP;

        break;

    default:
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
