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
#include <avr/wdt.h>
#include <os.h>
#include <os_avr.h>
#include <string.h>

/* Global interrupt level. */
/* TRUE: Interrupt Enabled
 * FALSE: Interrupt Disabled */
volatile uint8_t sys_interrupt_level = TRUE;

/* Current task pointer */
extern TASK *current_task;

/* AVR system stack. */
uint8_t system_stack[SYS_STACK_SIZE];
uint8_t *system_stack_pointer;

/* Flag to specify that we are in ISR context. */
uint8_t avr_in_isr = FALSE;

/*
 * ISR(__vector_default)
 * This function will stub out any rogue interrupts, and
 * will reset the system if triggered.
 */
ISR(__vector_default)
{
    /* Disable interrupts. */
    DISABLE_INTERRUPTS();

#if (AVR_HARD_RESET == TRUE)
    /* Configure PC.7 as output. */
    DDRC |= ((1 << 7));

    /* Trigger a reset by discharging reset capacitor. */
    PORTC &= (uint8_t)(~(1 << 7));
#else
    /* Trigger a soft reset using watch dog timer. */
    wdt_enable(WDTO_15MS);
#endif

    /* We should never return from this function. */
    while(1) ;

} /* ISR(__vector_default) */

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

    /* Restore the previous task's context. */
    RESTORE_CONTEXT();

} /* ISR(TIMER1_COMPA_vect, ISR_NAKED) */

/*
 * current_hardware_tick
 * @return: This function will return hardware system tick.
 * This function will return current hardware tick.
 */
uint64_t current_hardware_tick()
{
    /* Return hardware system tick. */
    return ((current_system_tick() * OCR1A) + TCNT1);

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

    (tcb->tos)--;                                   /* Push R16 on the stack. */

    *(tcb->tos) = 0x80;                             /* Push SREG on the stack. */
    (tcb->tos)--;

    *(tcb->tos) = 0x00;                             /* The compiler expects R1 to be 0. */
    (tcb->tos) -= 0x17;                             /* Push R1-R15, R0, R17-R23 on the stack. */

    *(tcb->tos) = ((uint16_t)argv & 0x00FF);
    (tcb->tos)--;                                   /* Push R24 on the stack. */

    *(tcb->tos) = ((uint16_t)argv & 0xFF00) >> 8;
    (tcb->tos)--;                                   /* Push R25 on the stack. */

    (tcb->tos) -= 0x06;                             /* Push R26-R31 on the stack. */

    *(tcb->tos) = TRUE;                             /* Push system interrupt level for this task. */
    (tcb->tos)--;

} /* os_stack_init */

/*
 * avr_stack_init
 * This function initializes system stack pointer.
 */
void avr_stack_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void avr_stack_init(void)
{
    /* Disable watch dog timer. */
    MCUSR = 0;
    wdt_disable();

#ifdef CONFIG_BOOTLOAD
    /* Perform boot load operation. */
    BOOTLOAD();
#endif

    /* Load a predefined pattern on the system stack. */
    memset(system_stack, CONFIG_STACK_PATTERN, SYS_STACK_SIZE);

    /* Set the stack pointer to the end of the system stack. */
    LOAD_SYSTEM_STACK();

} /* avr_stack_init */

#ifdef CONFIG_TASK_STATS
/*
 * avr_sys_stack_fill
 * This function will load a predefined pattern on the system stack.
 */
void avr_sys_stack_fill(void) __attribute__((naked)) __attribute__((section(".init8")));
void avr_sys_stack_fill(void)
{
    uint32_t i = 0;

    /* Load a predefined pattern on the system stack until we hit the
     * stack pointer. */
    while ((uint8_t *)SP > &system_stack[i])
    {
        /* Load a predefined pattern. */
        system_stack[i] = CONFIG_STACK_PATTERN;
        i++;
    }

} /* avr_sys_stack_fill */
#endif /* CONFIG_TASK_STATS */

/*
 * control_to_system
 */
NAKED_FUN control_to_system()
{
    /* If we are in a task. */
    if (avr_in_isr == FALSE)
    {
        /* Save the context on the current task's stack. */
        /* This will also disable global interrupts. */
        SAVE_CONTEXT();

        /* Load system stack. */
        LOAD_SYSTEM_STACK();
    }
    else
    {
        /* In interrupt save the context on the system stack. */
        SAVE_STACK();
    }

    /* If we do have a task to switch. */
    if (current_task != NULL)
    {
        /* Context has already been saved, just switch to new
         * task here. */
        set_current_task((TASK *)scheduler_get_next_task());
    }

    /* If we are in a task. */
    if (avr_in_isr == FALSE)
    {
        /* Restore the previous task's context. */
        RESTORE_CONTEXT();
    }
    else
    {
        /* Restore the interrupt context. */
        RESTORE_STACK();

        /* Return from this function. */
        RETURN_FUNCTION();
    }

} /* control_to_system */
