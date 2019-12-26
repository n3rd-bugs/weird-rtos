/*
 * avr.c
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
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <kernel.h>
#include <avr.h>

/* Global interrupt level. */
/* TRUE: Interrupt Enabled
 * FALSE: Interrupt Disabled */
volatile uint8_t sys_interrupt_level = TRUE;

/* AVR system stack. */
uint16_t system_stack_end;

/* Flag to specify that we are in ISR context. */
uint8_t avr_in_isr = FALSE;

/*
 * stack_init
 * @tcb: Task control block for which stack is needed to be initialized.
 * @entry: Task entry function.
 * @argv: Argument that is needed to be passed to the task.
 * This function initializes stack for a new task.
 */
void stack_init(TASK *tcb, TASK_ENTRY *entry, void *argv)
{
    /* The start of the task entry will be popped off the stack last, so place
     * it on first. */
    *(tcb->tos) = ((uint16_t)entry & 0x00FF);
    (tcb->tos)--;

    *(tcb->tos) = ((uint16_t)entry & 0xFF00) >> 8;
    (tcb->tos)--;

    /* Push R16 on the stack. */
    (tcb->tos)--;

    /* Push SREG on the stack. */
    *(tcb->tos) = 0x80;
    (tcb->tos)--;

    /* Push RAMPZ on the stack. */
    *(tcb->tos) = 0x00;
    (tcb->tos)--;

    /* The compiler expects R1 to be 0. */
    *(tcb->tos) = 0x00;

    /* Push R1-R15, R0, R17-R23 on the stack. */
    (tcb->tos) -= 0x17;

    /* Push R24 on the stack. */
    *(tcb->tos) = ((uint16_t)argv & 0x00FF);
    (tcb->tos)--;

    /* Push R25 on the stack. */
    *(tcb->tos) = ((uint16_t)argv & 0xFF00) >> 8;
    (tcb->tos)--;

    /* Push R26-R31 on the stack. */
    (tcb->tos) -= 0x06;

    /* Push system interrupt level for this task. */
    *(tcb->tos) = TRUE;
    (tcb->tos)--;

} /* stack_init */

/*
 * control_to_system
 */
NAKED_FUN control_to_system(void)
{
    /* Save context for either a task or an ISR. */
    SAVE_CONTEXT_CTS();

    /* If we are in a task. */
    if (avr_in_isr == FALSE)
    {
        /* We may switch to a new task so mark an exit. */
        MARK_EXIT();
    }

    /* If current task is in running state. */
    if (current_task->state == TASK_RUNNING)
    {
        /* Return the current task. */
        scheduler_task_yield(current_task, YIELD_SYSTEM);
    }

    /* If we are in process of suspending this task. */
    else if (current_task->state == TASK_TO_BE_SUSPENDED)
    {
        /* We just suspended this task. */
        current_task->state = TASK_SUSPENDED;
    }

    /* Context has already been saved, just switch to new
     * task here. */
    set_current_task((TASK *)scheduler_get_next_task());

    /* Set the current task as running. */
    current_task->state = TASK_RUNNING;

    /* If we are in a task. */
    if (avr_in_isr == FALSE)
    {
        /* Mark entry to a new task. */
        MARK_ENTRY();

        /* Restore the next task's context. */
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

/*
 * ISR(__vector_default)
 * This function will stub out any rogue interrupts, and
 * will reset the system if triggered.
 */
ISR(__vector_default, ISR_NAKED)
{
    CPU_ISR_ENTER();

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

    /* Wait for WDT interrupt to trigger. */
    while ((WDTCSR & (1 << WDIF)) == 0);

    /* Enable interrupts to handle WDT interrupt. */
    ENABLE_INTERRUPTS();
#endif

    /* We should never return from this function. */
    while(1) ;

    CPU_ISR_EXIT();

} /* ISR(__vector_default) */
