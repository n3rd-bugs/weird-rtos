/*
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose.
 */
#include <os.h>
#include <os_avr.h>

/* This will hold the control block for the currently running. */
TASK *current_task;

/* This is used for time keeping in the system. */
uint64_t current_tick = 0;
/*
 * os_process_system_tick
 * This function is called at each system tick. Here we decide which task is
 * needed to be run in next system tick.
 */
void os_process_system_tick()
{
    /* DEBUG: Set PB0 high. */
    PORTB &= ~(1);

    /* Increment system clock. */
    current_tick ++;

    /* Check if we can actually preempt the current task. */
    if (!(current_task->flags & TASK_DONT_PREEMPT))
    {
        /* If current task has a scheduler defined. */
        if (current_task->scheduler != NULL)
        {
            /* Re-enqueue/schedule this task in the scheduler. */
            ((SCHEDULER *)current_task->scheduler)->yield(current_task, YIELD_SYSTEM);
        }

        /* Get the task that should run next. */
        current_task = scheduler_get_next_task();
    }

    /* DEBUG: Set PB0 low. */
    PORTB |= (1);

    /* Return from this function. */
    RETURN_FUNCTION();

} /* os_process_system_tick */

/*
 * task_yield
 * This function is used to yield current task. This can be called from any
 * task. Depending on task priority the current task will be preempted or
 * continue to run after this is called.
 */
void task_yield()
{
    /* Save the context on the current task's stack. */
    /* This will also disable global interrupts. */
    SAVE_CONTEXT();

    /* If current task has a scheduler defined. */
    if (current_task->scheduler != NULL)
    {
        /* Re-enqueue/schedule this task in the scheduler. */
        ((SCHEDULER *)current_task->scheduler)->yield(current_task, YIELD_MANUAL);
    }

    /* Get the task that should run next. */
    current_task = scheduler_get_next_task();

    /* Restore the previous task's context. */
    RESTORE_CONTEXT();

    /* Return and enable global interrupts. */
    RETURN_ENABLING_INTERRUPTS();

} /* task_yield */

/*
 * task_waiting
 * This called when the current task is waiting for a resource and is needed
 * to be removed from the normal scheduling methods.
 */
void task_waiting()
{
    /* Save the context on the current task's stack. */
    /* This will also disable global interrupts. */
    SAVE_CONTEXT();

    /* We will not re-enqueue this task as it is suspended and only the
     * suspending component can resume this task. */

    /* Get the task that should run next. */
    current_task = scheduler_get_next_task();

    /* Restore the previous task's context. */
    RESTORE_CONTEXT();

    /* Return and enable global interrupts. */
    RETURN_ENABLING_INTERRUPTS();

} /* task_waiting */

/*
 * set_current_task
 * @tcb: The task control block that is needed to be set as current task.
 * This function is called when we need to set current task, usually required
 * by scheduling routines.
 */
void set_current_task(TASK *tcb)
{
    /* Set the current task to the given task. */
    current_task = tcb;

} /* set_current_task */

/*
 * get_current_task
 * return: The pointer to control block of the currently running task.
 * This function returns the pointer to the control block of the currently
 * running task.
 */
TASK *get_current_task()
{
    /* Return the current task's control block. */
   return (current_task);

} /* get_current_task */

/*
 * current_system_tick
 * return: Current system tick.
 * This function returns the number of system ticks elapsed from the system
 * boot.
 */
uint64_t current_system_tick()
{
    /* Return current system tick. */
   return (current_tick);

} /* current_system_tick */

/*
 * os_run
 * This function starts the operating system. In normal operation this function
 * should never return.
 */
void os_run()
{
    /* Initialize system clock. */
    system_tick_Init();

    /* Get the first task that is needed to run. */
    current_task = scheduler_get_next_task();

    /* Load/restore task's context. */
    RESTORE_CONTEXT();

    /* Return and enable global interrupts. */
    RETURN_ENABLING_INTERRUPTS();

} /* os_run */
