/*
 * kernel.c
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
#include <kernel.h>
#include <kernel_target.h>
#include <sleep.h>

/* This will hold the control block for the currently running. */
TASK *current_task = NULL;

/* This will hold the control block for task in which we will return after
 * executing the interrupt. */
TASK *return_task = NULL;

/* This is used for time keeping in the system. */
uint32_t current_tick = 0;

/*
 * task_yield
 * This function is used to yield the current task. This can be called from any
 * task. Depending on task priority the current task will be preempted or
 * continue to run after this is called. This function will also enable
 * interrupts when required.
 */
void task_yield(void)
{
    INT_LVL interrupt_level;
    uint8_t in_isr = FALSE;

    /* If we are in an interrupt. */
    if (current_task == NULL)
    {
        /* Return task should not be null. */
        ASSERT(return_task == NULL);

        /* Pick the return task. */
        current_task = return_task;

        /* We are in an interrupt. */
        in_isr = TRUE;
    }

    /* Check if we can actually yield the current task. */
    if (current_task->lock_count == 0)
    {
        /* Disable interrupts. */
        interrupt_level = GET_INTERRUPT_LEVEL();
        DISABLE_INTERRUPTS();

        /* Re-enqueue/schedule this task in the scheduler. */
        scheduler_task_yield(current_task, YIELD_MANUAL);

        /* Schedule next task and enable interrupts. */
        CONTROL_TO_SYSTEM();

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);
    }

    else
    {
        /* Set the flag that we need to process a context switch. */
        current_task->flags |= TASK_SCHED_DRIFT;
    }

    if (in_isr == TRUE)
    {
        /* Save the return task. */
        return_task = current_task;

        /* Clear current task. */
        current_task = NULL;
    }

} /* task_yield */

/*
 * current_system_tick
 * @return: Current system tick.
 * This function returns the number of system ticks elapsed from the system
 * boot.
 */
uint32_t current_system_tick(void)
{
    uint32_t return_tick;
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Atomically save the current system tick. */
    return_tick = current_tick;

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Return current system tick. */
   return (return_tick);

} /* current_system_tick */

/*
 * kernel_run
 * This function starts the operating system. In normal operation this function
 * should never return.
 */
void kernel_run(void)
{
#ifndef SYS_LOG_NONE
    /* Initialize system logging. */
    sys_log_init();
#endif

    /* Initialize system clock. */
    system_tick_Init();

    /* Get the first task that is needed to run. */
    current_task = scheduler_get_next_task();

    /* Load/restore task's context. */
    RESTORE_CONTEXT_FIRST();

} /* kernel_run */
