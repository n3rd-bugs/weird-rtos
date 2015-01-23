/*
 * os.c
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
#include <os_target.h>

/* This will hold the control block for the currently running. */
TASK *current_task = NULL;

/* This is used for time keeping in the system. */
uint64_t current_tick = 0;
/*
 * os_process_system_tick
 * This function is called at each system tick. Here we decide which task is
 * needed to run in next system tick.
 */
void os_process_system_tick()
{
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

} /* os_process_system_tick */

/*
 * task_yield
 * This function is used to yield the current task. This can be called from any
 * task. Depending on task priority the current task will be preempted or
 * continue to run after this is called. This function will also enable
 * interrupts when required.
 */
void task_yield()
{
    uint32_t interrupt_level;

    OS_ASSERT(current_task == NULL);

    /* Check if we can actually yield the current task. */
    if (!(current_task->flags & TASK_DONT_PREEMPT))
    {
        /* Disable interrupts. */
        interrupt_level = GET_INTERRUPT_LEVEL();
        DISABLE_INTERRUPTS();

        /* If current task has a scheduler defined. */
        if (current_task->scheduler != NULL)
        {
            /* Re-enqueue/schedule this task in the scheduler. */
            ((SCHEDULER *)current_task->scheduler)->yield(current_task, YIELD_MANUAL);
        }

        /* Schedule next task and enable interrupts. */
        CONTROL_TO_SYSTEM();

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);
    }

} /* task_yield */

/*
 * task_waiting
 * This called when the current task is waiting for a resource and is needed
 * to be removed from the parent scheduling class, and will be rescheduled by
 * the resource manager. If required user can keep the interrupts disabled when
 * jumping into this function. Interrupts will be enabled when control is
 * needed to be returned to the system.
 */
void task_waiting()
{
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable interrupts. */
    DISABLE_INTERRUPTS();

    /* We will not re-enqueue this task as it is suspended and only the
     * suspending component can resume this task. */

    /* Give control back to system and enable interrupts. */
    CONTROL_TO_SYSTEM();

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

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
 * @return: The pointer to control block of the currently running task.
 * This function returns the pointer to the control block of the current task.
 */
TASK *get_current_task()
{
    /* Assert if current task is null. */
    OS_ASSERT(current_task == NULL);

    /* Return the current task's control block. */
   return (current_task);

} /* get_current_task */

/*
 * current_system_tick
 * @return: Current system tick.
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
    RESTORE_CONTEXT_FIRST();

} /* os_run */
