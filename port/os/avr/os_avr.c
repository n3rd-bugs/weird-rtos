/*
 * os_avr.c
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
#include <avr/interrupt.h>
#include <os.h>
#include <os_avr.h>

/* Global interrupt level. */
/* TRUE: Interrupt Enabled
 * FALSE: Interrupt Disabled */
volatile uint32_t sys_interrupt_level = TRUE;

/* Local variable definitions. */
/* If we have just did a forced tick. */
static volatile uint8_t force_tick = FALSE;
static TASK *next_task;

/* Current task pointer */
extern TASK *current_task;

/*
 * ISR(TIMER1_COMPA_vect, ISR_NAKED)
 * This is timer interrupt that will be called at each system tick.
 */
ISR(TIMER1_COMPA_vect, ISR_NAKED)
{
    /* Save the context on the current task's stack. */
    /* This will also disable global interrupts. */
    SAVE_CONTEXT();

    /* If this was not a forced tick. */
    if (force_tick == FALSE)
    {
        /* Process system tick. */
        os_process_system_tick();

        /* Check if we can actually preempt the current task. */
        if (current_task->lock_count == 0)
        {
            /* If current task has a scheduler defined. */
            if (current_task->scheduler != NULL)
            {
                /* Re-enqueue/schedule this task in the scheduler. */
                ((SCHEDULER *)current_task->scheduler)->yield(current_task, YIELD_SYSTEM);
            }

            /* Get and set the task that should run next. */
            set_current_task(scheduler_get_next_task());
        }

        else
        {
            /* Set the flag that we need to process a context switch. */
            current_task->flags |= TASK_SCHED_DRIFT;
        }
    }
    else
    {
        /* Get and set the task that should run next. */
        set_current_task(next_task);

        /* We have processed a forced tick. */
        force_tick = FALSE;
    }

    /* Restore the previous task's context. */
    RESTORE_CONTEXT();

    /* Return and enable global interrupts. */
    RETURN_ENABLING_INTERRUPTS();

} /* ISR(TIMER1_COMPA_vect, ISR_NAKED) */

/*
 * current_hardware_tick
 * @return: This function will return hardware system tick.
 * This function will return current hardware tick.
 */
uint64_t current_hardware_tick()
{
    /* Return hardware system tick. */
    return ((current_system_tick() << 16) + TCNT1);

} /* current_hardware_tick */

/*
 * system_tick_Init
 * This initializes system tick. On AVR we are using Timer1 with CCP1 module to
 * reload the timer when expired.
 */
void system_tick_Init()
{
    /* Using 16bit timer 1 to generate the system tick. */
    TCNT1 = 0x00;
    OCR1A = (((SYS_FREQ / OS_TICKS_PER_SEC / 64) - 1) & 0xFFFF);

    /* Setup clock source and compare match behavior. */
    TCCR1B =  0x03 | 0x08;

    /* Enable the compare A match interrupt. */
    TIMSK1 |= 0x02;

} /* system_tick_Init */

/*
 * os_stack_init
 * @tcb: Task control block for which stack is needed to be initialized.
 * @entry: Task entry function.
 * @argv: Argument that is needed to be passed to the task.
 * This function initializes stack for a new task.
 */
void os_stack_init(TASK *tcb, TASK_ENTRY *entry, void *argv)
{
    /* The start of the task code will be popped off the stack last, so place
     * it on first. */
    *(tcb->tos) = ((uint16_t)entry & 0x00FF);
    (tcb->tos)--;

    *(tcb->tos) = ((uint16_t)entry & 0xFF00) >> 8;
    (tcb->tos)--;

    (tcb->tos)--;                                   /* Push R0 on the stack. */

    *(tcb->tos) = 0x80;
    (tcb->tos)--;

    *(tcb->tos) = 0x00;                             /* The compiler expects R1 to be 0. */
    (tcb->tos) -= 0x17;                             /* Push R1-R23 on the stack. */

    *(tcb->tos) = ((uint16_t)argv & 0x00FF);
    (tcb->tos)--;                                   /* Push R24 on the stack. */

    *(tcb->tos) = ((uint16_t)argv & 0xFF00) >> 8;
    (tcb->tos)--;                                   /* Push R25 on the stack. */

    (tcb->tos) -= 0x06;                             /* Push R26-R31 on the stack. */

} /* os_stack_init */

/*
 * control_to_system
 */
void control_to_system()
{
    uint16_t timer_value;

    /* If we are not in an ISR. */
    if (get_current_task() != NULL)
    {
        /* Get next task we need to run. */
        next_task = scheduler_get_next_task();

        /* If we really need to schedule a context switch. */
        if (next_task != current_task)
        {
            /* Save current timer tick. */
            timer_value = TCNT1;

            /* This will be a forced interrupt. */
            force_tick = TRUE;

            /* Trigger a forced timer interrupt. */
            OCR1A = (TCNT1 + 1);

            /* Wait for timer interrupt to trigger. */
            while ((TIFR1 & 0x02) == 0);

            /* Restore the timer tick and compare register. */
            TCNT1 = timer_value;
            OCR1A = (((SYS_FREQ / OS_TICKS_PER_SEC / 64) - 1) & 0xFFFF);

            /* If we are in an ISR then we don't want to enable interrupts here
             * as it might cause a nested interrupt. */
            if (return_task == NULL)
            {
                /* Enable interrupts. */
                ENABLE_INTERRUPTS();
            }
        }
    }

} /* control_to_system */
