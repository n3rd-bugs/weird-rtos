/*
 * cortex_m0.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
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
    init_stack_frame->psr   = (uint32_t)(0x01000000);

    /* Push a dummy software stack frame. */
    tcb->tos -= sizeof(software_stack_farme);

} /* stack_init */

/*
 * run_first_task
 * This is responsible for running first task.
 */
void run_first_task(void)
{
    /* Set default interrupt level as disabled. */
    sys_interrupt_level = FALSE;

    /* Get the task needed to run. */
    current_task = scheduler_get_next_task();

    /* Set PendSV and System Tick interrupt priorities to avoid nested
     * interrupts. */
    CORTEX_M0_SET_PENDSV_PRI();

    /* Mark entry to a new task. */
    MARK_ENTRY();

    /* Set default interrupt level as enabled. */
    sys_interrupt_level = TRUE;

    /* Invoke service call to start first task. */
    asm volatile
    (
    "   CPSIE       I           \r\n"   /* Enable all interrupts. */
    "   CPSIE       F           \r\n"
    "   DSB                     \r\n"   /* Memory barrier. */
    "   ISB                     \r\n"
    "   SVC         0           \r\n"   /* Invoke service call to start first task. */
    );

    /* Should never get here. */
    ASSERT(TRUE);

} /* run_first_task */

/*
 * control_to_system
 */
void control_to_system(void)
{
    INT_LVL int_level;

    /* If we are not in an ISR. */
    if (return_task == NULL)
    {
        /* Mark entry to a new task. */
        MARK_ENTRY();
    }

    /* Get interrupt level. */
    int_level = GET_INTERRUPT_LEVEL();

    /* Enable interrupts. */
    ENABLE_INTERRUPTS();

    /* Schedule a context switch. */
    PEND_SV();

    /* Restore interrupt level. */
    SET_INTERRUPT_LEVEL(int_level);

    /* If we are not in an ISR. */
    if (return_task == NULL)
    {
        /* Again mark an exit. */
        MARK_EXIT();
    }

} /* control_to_system */

#ifdef CONFIG_SLEEP
/*
 * isr_sysclock_handle
 * This is system tick interrupt handle.
 */
ISR_FUN isr_sysclock_handle(void)
{
    /* Disable other interrupts. */
    DISABLE_INTERRUPTS();

    /* Process system tick. */
    if (process_system_tick())
    {
        /* Check if we can actually preempt the current task. */
        if (current_task->lock_count == 0)
        {
            /* We should never have a task here with state not running. */
            ASSERT(current_task->state != TASK_RUNNING);

            /* Schedule a context switch. */
            PEND_SV();
        }
        else
        {
            /* Set the flag that we need to process a context switch. */
            current_task->flags |= TASK_SCHED_DRIFT;
        }
    }

    /* Enable interrupts. */
    ENABLE_INTERRUPTS();

} /* isr_sysclock_handle */
#endif /* CONFIG_SLEEP */

/*
 * isr_sv_handle
 * This SV interrupt handle.
 */
NAKED_ISR_FUN isr_sv_handle(void)
{
    /* Load context for the first task. */
    asm volatile
    (
    "   LDR         R1, =current_task       \r\n"   /* Load task pointer. */
    "   LDR         R1, [R1, #0]            \r\n"
    "   MOV         R2, %[tos_offset]       \r\n"   /* Load TOS pointer. */
    "   LDR         R0, [R1, R2]            \r\n"   /* Load TOS. */
    "   LDMIA       R0!, {R4 - R7}          \r\n"   /* Load R8 - R11 from stack. */
    "   MOV         R8, R4                  \r\n"   /* Save R8 - R11. */
    "   MOV         R9, R5                  \r\n"
    "   MOV         R10, R6                 \r\n"
    "   MOV         R11, R7                 \r\n"
    "   LDMIA       R0!, {R4 - R7}          \r\n"   /* Load R4 - R7 from stack. */
    "   MSR         PSP, R0                 \r\n"   /* Load PSP. */
    "   ISB                                 \r\n"   /* Memory barrier. */
    "   LDR         R0, =0xFFFFFFFD         \r\n"   /* Load R0 with return from exception. */
    "   MOV         LR, R0                  \r\n"   /* Load LR. */
    "   BX          LR                      \r\n"
    :: [tos_offset] "I" (OFFSETOF(TASK, tos))
    );

} /* isr_sv_handle */

/*
 * isr_pendsv_handle
 * This pendSV interrupt handle. This should not nest an interrupt.
 */
NAKED_ISR_FUN isr_pendsv_handle(void)
{
    /* Save current task's context. */
    asm volatile
    (
    "   CPSID       I                       \r\n"   /* Disable interrupts. */
    "   MOV         R0, SP                  \r\n"   /* Save SP in R0. */
    "   PUSH        {R0, LR}                \r\n"   /* Save R0 and LR in stack. */
    "   MRS         R0, PSP                 \r\n"   /* Load PSP in R0. */
    "   ISB                                 \r\n"
    "   SUB         R0, #16                 \r\n"   /* Move R3 forward in stack for 4 registers. */
    "   STMIA       R0!, {R4, R5, R6, R7}   \r\n"   /* Save R4 - R7 in stack. */
    "   MOV         R4, R8                  \r\n"   /* Save R8 - R11 in R4 - R7. */
    "   MOV         R5, R9                  \r\n"
    "   MOV         R6, R10                 \r\n"
    "   MOV         R7, R11                 \r\n"
    "   SUB         R0, #32                 \r\n"   /* Move R3 forward in stack past the last 4 registers. */
    "   STMIA       R0!, {R4, R5, R6, R7}   \r\n"   /* Save R8 - R11 in stack. */
    "   SUB         R0, #16                 \r\n"   /* Move R3 to the new top of the stack. */
    "   LDR         R1, =current_task       \r\n"   /* Load current task pointer. */
    "   LDR         R1, [R1, #0]            \r\n"
    "   MOV         R2, %[tos_offset]       \r\n"   /* Load TOS offset for the TCB. */
    "   STR         R0, [R1, R2]            \r\n"   /* Save the TOS in the TCB. */
    :: [tos_offset] "I" (OFFSETOF(TASK, tos))
    );

    /* We are switching to a new task so mark an exit. */
    MARK_EXIT();

#ifdef TASK_STATS
    /* Break the task stack pattern. */
    *(current_task->tos - 1) = 0x00;
#endif /* TASK_STATS */

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

    /* Set global interrupt level to be disabled so we don't end-up enabling
     * interrupts when we are are not supposed to. */
    sys_interrupt_level = FALSE;

    /* Get the next task needed to run. */
    current_task = scheduler_get_next_task();

    /* Set the current task as running. */
    current_task->state = TASK_RUNNING;

    /* Set global interrupt level to be enabled. */
    sys_interrupt_level = TRUE;

    /* Mark entry to a new task. */
    MARK_ENTRY();

    /* Load context for current task. */
    asm volatile
    (
    "   LDMIA   %[sp]!, {R4 - R7}           \r\n"   /* Load R8 - R11 from stack. */
    "   MOV     R8, R4                      \r\n"   /* Save R8 - R11. */
    "   MOV     R9, R5                      \r\n"
    "   MOV     R10, R6                     \r\n"
    "   MOV     R11, R7                     \r\n"
    "   LDMIA   %[sp]!, {R4 - R7}           \r\n"   /* Load R4 - R7 from stack. */
    "   MSR     PSP, %[sp]                  \r\n"   /* Load PSP. */
    ::
    [sp] "r" (current_task->tos)
    );

    /* Enable interrupts and return from this function. */
    asm volatile
    (
    "   POP         {R0, R1}                \r\n"   /* Load R0 and LR. */
    "   MOV         SP, R0                  \r\n"   /* Update SP. */
    "   CPSIE       I                       \r\n"   /* Enable interrupts. */
    "   ISB                                 \r\n"
    "   BX          R1                      \r\n"   /* Return. */
    );

} /* isr_pendsv_handle */
