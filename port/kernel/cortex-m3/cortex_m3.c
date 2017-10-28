/*
 * cortex_m3.c
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

/* Global interrupt level. */
/* TRUE: Interrupt Enabled
 * FALSE: Interrupt Disabled */
volatile INT_LVL sys_interrupt_level = TRUE;

/* Local variable definitions. */
/* We need to keep this to save the stack pointer for the task we are
 * switching from. */
/* Removing this requires implementation of naked ISR functions. */
static TASK *last_task;

/*
 * stack_init
 * @tcb: Task control block needed to be initialized.
 * @task_entry: Task entry function.
 * @argv: Arguments to be passed to the entry function.
 * This function is responsible for initializing stack for a tack.
 */
void stack_init(TASK *tcb, TASK_ENTRY *task_entry, void *argv)
{
    hardware_stack_farme *init_stack_frame;

    /* Push a hardware stack frame on the stack. */
    tcb->tos -= sizeof(hardware_stack_farme);

    /* Initialize stack frame, this will be loaded when CPU
     * the will jump in the task first time. */
    init_stack_frame        = (hardware_stack_farme *)(tcb->tos);
    init_stack_frame->r0    = (uint32_t)argv;
    init_stack_frame->r1    = 0;
    init_stack_frame->r2    = 0;
    init_stack_frame->r3    = 0;
    init_stack_frame->r12   = 0;
    init_stack_frame->pc    = (uint32_t)task_entry;
    init_stack_frame->lr    = (uint32_t)0x0;
    init_stack_frame->psr   = INITIAL_XPSR;

    /* Push a dummy software stack frame. */
    tcb->tos -= sizeof(software_stack_farme);

} /* stack_init */

/*
 * run_first_task
 * This is responsible for running first task.
 */
void run_first_task(void)
{
    /* We are running the first task so there is no last task. */
    last_task = NULL;

    /* Set default interrupt level as disabled. */
    sys_interrupt_level = FALSE;

    /* Set PendSV and System Tick interrupt priorities to avoid nested
     * interrupts. */
    CORTEX_M3_INT_PEND_SV_PRI_REG = CORTEX_M3_INT_SYS_PRI;
    CORTEX_M3_INT_SYS_TICK_PRI_REG = CORTEX_M3_INT_SYS_PRI;

    /* Schedule a context switch. */
    PEND_SV();

    __asm volatile
    (
    "   LDR     R0, =0xE000ED08   \n"
    "   LDR     R0, [R0]          \n"
    "   LDR     R0, [R0]          \n"
    "   MSR     MSP, R0           \n"
    );

    /* Mark entry to a new task. */
    MARK_ENTRY();

    /* Enable interrupts. */
    ENABLE_INTERRUPTS();

    /* Should never get here. */
    ASSERT(TRUE);

} /* run_first_task */

/*
 * control_to_system
 */
void control_to_system(void)
{
    /* If we have not already scheduled a context switch. */
    if (last_task == NULL)
    {
        /* We may switch to a new task so mark an exit. */
        MARK_EXIT();

        /* Save the task from which we will be switching. */
        last_task = current_task;

        /* Process and get the next task in this task's context. */
        current_task = scheduler_get_next_task();

        /* Check if we need to switch context. */
        if (current_task != last_task)
        {
            /* Mark entry to a new task. */
            MARK_ENTRY();

            /* Schedule a context switch. */
            PEND_SV();

            /* Enable interrupts. */
            ENABLE_INTERRUPTS();

            /* Again mark an exit. */
            MARK_EXIT();
        }

        else
        {
            /* We are not scheduling a context switch. */
            last_task = NULL;
        }

        /* Mark entry to a new task. */
        MARK_ENTRY();
    }

} /* control_to_system */

/*
 * isr_sysclock_handle
 * This is system tick interrupt handle.
 */
ISR_FUN isr_sysclock_handle(void)
{
    /* Process system tick. */
    process_system_tick();

    /* We may switch to a new task so mark an exit. */
    MARK_EXIT();

    /* If we have not already scheduled a context switch. */
    if (last_task == NULL)
    {
        /* Check if we can actually preempt the current task. */
        if (current_task->lock_count == 0)
        {
            /* Save the current task pointer. */
            last_task = current_task;

            /* Re-enqueue/schedule this task in the scheduler. */
            scheduler_task_yield(current_task, YIELD_MANUAL);

            /* Get the task that should run next. */
            current_task = scheduler_get_next_task();

            /* Check if we need to switch context. */
            if (current_task != last_task)
            {
                /* Schedule a context switch. */
                PEND_SV();
            }

            else
            {
                /* We are not scheduling a context switch. */
                last_task = NULL;
            }
        }

        else
        {
            /* Set the flag that we need to process a context switch. */
            current_task->flags |= TASK_SCHED_DRIFT;
        }
    }

    /* Mark entry to a new task. */
    MARK_ENTRY();

} /* isr_sysclock_handle */

/*
 * isr_pendsv_handle
 * This pendSV interrupt handle.
 */
NAKED_ISR_FUN isr_pendsv_handle(void)
{
    register int *stack asm ("r2");

    /* Disable global interrupts. */
    asm volatile
    (
    "   CPSID       I                   \r\n"
    );

    /* Check if need to save last task context. */
    if (last_task)
    {
        /* Force usage of R2 to store the stack pointer. */
        stack = (int *)last_task->tos;

        /* Save last task context on stack. */
        asm volatile
        (
        "   MRS     %[sp], PSP          \r\n"
        "   STMDB   %[sp]!, {R4 - R11}  \r\n"
        :
        [sp] "=r" (stack)
        );

        /* Save the stack pointer. */
        last_task->tos = (uint8_t *)stack;

#ifdef TASK_STATS
        /* Break the task stack pattern. */
        *(last_task->tos - 1) = 0x00;
#endif /* TASK_STATS */
    }

    /* Clear the last task. */
    last_task = NULL;

    /* Load context for new task. */
    asm volatile
    (
    "   LDMFD   %[sp]!, {R4 - R11}      \r\n"
    "   MSR     PSP, %[sp]              \r\n"
    "   ORR     LR, LR, #0x04           \r\n"
    ::
    [sp] "r" (current_task->tos)
    );

    /* Clear the priority mask to enable other interrupts. */
    asm volatile
    (
    "   MOVS        R0, #0              \r\n"
    "   MSR         BASEPRI, R0         \r\n"
    );

    /* Just enable system tick interrupt. */
    CORTEX_M3_SYS_TICK_REG |= CORTEX_M3_SYS_TICK_MASK;

    /* Enable interrupts and return from this function. */
    asm volatile
    (
    "   DSB                             \r\n"
    "   ISB                             \r\n"
    "   CPSIE       I                   \r\n"
    "   BX          LR                  \r\n"
    );

} /* isr_pendsv_handle */
