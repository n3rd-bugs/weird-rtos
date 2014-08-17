#include <sleep.h>
#include <os.h>
#include <sll.h>

#ifdef CONFIG_INCLUDE_SLEEP

static uint8_t sleep_task_sort(void *node, void *task)
{
    uint8_t schedule = FALSE;

    /* Check if task is needed to be scheduled before the given task . */
    if ( (((TASK *)node)->tick_sleep > ((TASK *)task)->tick_sleep) ||

         /* Check if this node task has same scheduling instance as the given task. */
         ( (((TASK *)node)->tick_sleep == ((TASK *)task)->tick_sleep) &&

           /* Schedule given task before the node if it has higher priority. */
           (((TASK *)node)->priority > ((TASK *)task)->priority) ) )
    {
        /* Schedule the given task before this node. */
        schedule = TRUE;
    }

    /* Return if we need to schedule this task before the given node. */
    return (schedule);

} /* sleep_task_sort. */

static TASK *sleep_process_system_tick(void)
{
    TASK *tcb = 0;

    /* Check if we need to schedule a sleeping task. */
    if ( (sleep_scheduler.ready_tasks.head != 0) &&
         (current_system_tick() >= sleep_scheduler.ready_tasks.head->tick_sleep) )
    {
        /* Schedule this sleeping task. */
        tcb = (TASK *)sll_pop(&sleep_scheduler.ready_tasks, OFFSETOF(TASK, next_sleep));

        /* Task is being resumed from sleep. */
        tcb->status = TASK_RESUME_SLEEP;
    }

    /* Return the task that is needed to be scheduled. */
    return (tcb);

} /* sleep_process_system_tick */

static void sleep_task_re_enqueue(TASK *tcb, uint8_t from)
{
    /* Process all the cases from a task can be re/scheduled. */
    switch (from)
    {
    case YIELD_CANNOT_RUN:
        /* Just put back this task on the scheduler list. */
        sll_push(&sleep_scheduler.ready_tasks, tcb, OFFSETOF(TASK, next_sleep));

        /* Task is being suspended. */
        tcb->status = TASK_SUSPENDED;

        break;
    default:
        break;
    }

} /* sleep_task_re_enqueue */

void sleep_add_to_list(TASK *tcb, uint32_t ticks)
{
    /* Calculate system tick at which task will be invoked. */
    tcb->tick_sleep = current_system_tick() + ticks;

    /* Put this task on the list of sleeping tasks. */
    /* This will also schedule this task. */
    sll_insert(&sleep_scheduler.ready_tasks, tcb, &sleep_task_sort, OFFSETOF(TASK, next_sleep));

} /* sleep_add_to_list */

void sleep_remove_from_list(TASK *tcb)
{
    /* Remove this task from the list of sleeping tasks. */
    sll_remove(&sleep_scheduler.ready_tasks, tcb, OFFSETOF(TASK, next_sleep));

} /* sleep_remove_from_list */

void sleep(uint32_t ticks)
{
    TASK *tcb;

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Save the current task pointer. */
    tcb = get_current_task();

    /* Add current task to the sleep list. */
    sleep_add_to_list(tcb, ticks);

    /* Task is being suspended. */
    tcb->status = TASK_SUSPENDED;

    /* Return control to the system.
     * We will resume from here when our required delay has been achieved. */
    task_waiting();

} /* sleep */

/* This defines members for sleep scheduling class. */
SCHEDULER sleep_scheduler =
{
    /* List of tasks that are enqueued to run. */
    .ready_tasks    = {0, 0},

    /* Function that will return the next task that is needed to run. */
    .get_task       = &sleep_process_system_tick,

    /* Function that will re-enqueue a given task. */
    .yield          = &sleep_task_re_enqueue,

    /* Priority for this scheduler, this should be less than aperiodic tasks. */
    .priority       = CONFIG_SLEEP_PIORITY
};

#else

void sleep(uint32_t ticks)
{
    /* Remove some warnings. */
    UNUSED_PARAM(ticks);

    /* If sleep is not enabled in the system then just yield the current task. */
    task_yield();

} /* sleep */

#endif /* CONFIG_INCLUDE_SLEEP */
