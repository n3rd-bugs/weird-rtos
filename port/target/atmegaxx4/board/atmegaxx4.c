/*
 * atmegaxx4.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#include <avr/interrupt.h>
#include <kernel.h>
#include <avr.h>

#ifdef CONFIG_SLEEP
/*
 * system_tick_Init
 * This initializes system tick. On AVR we are using Timer1 with CCP1 module to
 * reload the timer when expired.
 */
void system_tick_Init(void)
{
    /* Using 16bit timer 1 to generate the system tick. */
    TCNT1 = 0x0;
    OCR1A = (((SYS_FREQ / SOFT_TICKS_PER_SEC / 64) - 1) & 0xFFFF);

    /* Setup clock source and compare match behavior. */
    TCCR1B =  0x3 | 0x8;

    /* Enable the compare A match interrupt. */
    TIMSK1 |= 0x2;

} /* system_tick_Init */

/*
 * current_hardware_tick
 * @return: This function will return hardware system tick.
 * This function will return current hardware tick.
 */
uint64_t current_hardware_tick(void)
{
    /* Return hardware system tick. */
    return (((uint64_t)(current_system_tick() + (uint64_t)((TIFR1 & (1 << OCF1A)) ? 1 : 0)) * (uint64_t)(SYS_FREQ / SOFT_TICKS_PER_SEC / 64)) + (uint64_t)TCNT1);
} /* current_hardware_tick */

/*
 * ISR(TIMER1_COMPA_vect, ISR_NAKED)
 * This is timer interrupt that will be called at each system tick.
 */
ISR(TIMER1_COMPA_vect, ISR_NAKED)
{
    /* Save the context on the current task's stack. */
    /* This will also disable global interrupts. */
    SAVE_CONTEXT_ISR();

    /* Load system stack. */
    LOAD_SYSTEM_STACK();

    /* Clear the interrupt level. */
    sys_interrupt_level = 0;

    /* Process system tick. */
    if (process_system_tick())
    {
        /* We may switch to a new task so mark an exit. */
        MARK_EXIT();

        /* Check if we can actually preempt the current task. */
        if (current_task->lock_count == 0)
        {
            /* We should never have a task here with state not running. */
            ASSERT(current_task->state != TASK_RUNNING);

            /* Re-enqueue/schedule this task in the scheduler. */
            scheduler_task_yield(current_task, YIELD_SYSTEM);

            /* Get and set the task that should run next. */
            set_current_task(scheduler_get_next_task());

            /* Set the current task as running. */
            current_task->state = TASK_RUNNING;
        }

        else
        {
            /* Set the flag that we need to process a context switch. */
            current_task->flags |= TASK_SCHED_DRIFT;
        }

        /* Mark entry to a new task. */
        MARK_ENTRY();
    }

    /* Restore the previous task's context. */
    RESTORE_CONTEXT();

} /* ISR(TIMER1_COMPA_vect, ISR_NAKED) */
#endif /* CONFIG_SLEEP */
