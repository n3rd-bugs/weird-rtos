/*
 * os_cortex_m3.c
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

/* Global interrupt level. */
/* TRUE: Interrupt Enabled
 * FALSE: Interrupt Disabled */
uint32_t sys_interrupt_level = TRUE;

uint32_t clock_64_high_32 = 0;

/* Local variable definitions. */
/* We need to keep this to save the stack pointer for the task we are
 * switching from. */
/* Removing this requires implementation of naked ISR functions. */
static TASK *last_task;

/*
 * os_stack_init
 * @tcb: Task control block needed to be initialized.
 * @task_entry: Task entry function.
 * @argv: Arguments to be passed to the entry function.
 * This function is responsible for initializing stack for a tack.
 */
void os_stack_init(TASK *tcb, TASK_ENTRY *task_entry, void *argv)
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

} /* os_stack_init */

/*
 * system_tick_Init
 * This is responsible for initializing system timer tick. This will not
 * enable the interrupt as it will be enabled after first context switch.
 */
void system_tick_Init()
{
    /* Configure system tick to interrupt at the requested rate. */
    SYST_RVR = ( PCLK_FREQ / OS_TICKS_PER_SEC ) - 0x1;

    /* Enable system tick, enable system tick interrupt and use CPU clock
     * as it's source. */
    SYST_CSR = ( SysTick_CSR_ENABLE_MASK    |
                 SysTick_CSR_CLKSOURCE_MASK);

    /* PIT Configuration. */
    /* Enable global PIT clock. */
    SIM_SCGC6 = SIM_SCGC6_PIT_MASK;

    /* Enable PIT clock and run in debug mode. */
    PIT_MCR = PIT_MCR_FRZ_MASK;

    /* Disable timer. */
    PIT_TCTRL0 &= (~PIT_TCTRL_TEN_MASK);

    /* Set timer's count down value. */
    PIT_LDVAL0 = (uint32_t)(0xFFFFFFFF);

    /* If interrupt flag is already set. */
    if (PIT_TFLG0 & PIT_TFLG_TIF_MASK)
    {
        /* Clear the interrupt flag. */
        PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
    }

    /* Enable timer and it's interrupt. */
    PIT_TCTRL0 = (PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK);

    /* Enable PIT interrupt. */
    NVICISER2 |= (1 << ((INT_PIT0 - 0x10) % 32));

} /* system_tick_Init */

/*
 * pit_get_clock
 * This returns value of a 64-bit hardware timer running at peripheral frequency.
 */
uint64_t pit_get_clock()
{
    /* Load the soft-clock value. */
    uint64_t c_val = (uint32_t)clock_64_high_32;

    /* Make it higher word. */
    c_val = (c_val << 31);

    /* Load the hard-clock value. */
    c_val += (uint32_t)(0xFFFFFFFF - PIT_CVAL0);

    /* Return the  */
    return (c_val);

} /* pit_get_clock */

/*
 * run_first_task
 * This is responsible for running first task.
 */
void run_first_task()
{
    /* Disable interrupts. */
    DISABLE_INTERRUPTS();

    /* We are running the first task so there is no last task. */
    last_task = NULL;

    /* Set default interrupt level as disabled. */
    sys_interrupt_level = FALSE;

    /* Schedule a context switch. */
    PEND_SV();

    __asm volatile
    (
    "   LDR     R0, =0xE000ED08   \n"
    "   LDR     R0, [R0]          \n"
    "   LDR     R0, [R0]          \n"
    "   MSR     MSP, R0           \n"
    );

    /* Enable interrupts. */
    ENABLE_INTERRUPTS();

} /* run_first_task */

/*
 * control_to_system
 */
void control_to_system()
{
    /* Save the task from which we will be switching. */
    last_task = current_task;

    /* Process and get the next task in this task's context. */
    current_task = scheduler_get_next_task();

    /* Check if we need to switch context. */
    if (current_task != last_task)
    {
        /* Schedule a context switch. */
        PEND_SV();
    }

    /* Enable interrupts. */
    ENABLE_INTERRUPTS();

} /* run_first_task */

/*
 * isr_sysclock_handle
 * This is system tick interrupt handle.
 */
ISR_FUN isr_sysclock_handle(void)
{
    /* Disable interrupts. */
    DISABLE_INTERRUPTS();

    /* Save the current task pointer. */
    last_task = current_task;

    /* Process system tick. */
    os_process_system_tick();

    /* Check if we need to switch context. */
    if (current_task != last_task)
    {
        /* Schedule a context switch. */
        PEND_SV();
    }

    /* Enable interrupts. */
    ENABLE_INTERRUPTS();

} /* isr_sysclock_handle */

/*
 * isr_pendsv_handle
 * This pendSV interrupt handle.
 */
NAKED_ISR_FUN isr_pendsv_handle(void)
{
    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Check if need to save last task context. */
    if (last_task)
    {
        /* Save last task context on stack. */
        asm volatile
        (
        "   MRS     %[sp], PSP              \r\n"
        "   STMDB   %[sp]!, {R4 - R11}      \r\n"
        :
        [sp] "=r" (last_task->tos)
        );

#ifdef CONFIG_TASK_STATS
        /* Break the task stack pattern. */
        *(last_task->tos - 1) = 0x00;
#endif /* CONFIG_TASK_STATS */
    }

    /* Load context for new task. */
    asm volatile
    (
    "   LDMFD   %[sp]!, {R4 - R11}      \r\n"
    "   MSR     PSP, %[sp]              \r\n"
    "   ORR     LR, LR, #0x04           \r\n"
    ::
    [sp] "r" (current_task->tos)
    );

    /* Just enable system tick interrupt. */
    SYST_CSR |= (SysTick_CSR_TICKINT_MASK);

    /* Enable interrupts and return from this function. */
    RETURN_ENABLING_INTERRUPTS();

} /* isr_pendsv_handle */

/*
 * isr_servicecall_handle
 * This pendSV interrupt handle.
 */
ISR_FUN isr_clock64_tick(void)
{
    /* Timer roll over. */
    clock_64_high_32++;

    /* Clear the interrupt flag. */
    PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
    PIT_TCTRL0 = (PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK);

} /* isr_clock64_tick */

/*
 * cpu_interrupt
 * Default ISR callback.
 */
ISR_FUN cpu_interrupt(void)
{
    /* Assert the system. */
    OS_ASSERT(TRUE);

} /* cpu_interrupt */
/*
 * nmi_interrupt
 * NMI interrupt callback.
 */
ISR_FUN nmi_interrupt(void)
{
    /* Assert the system. */
    OS_ASSERT(TRUE);

} /* nmi_interrupt */
/*
 * hard_fault_interrupt
 * Hard fault interrupt callback.
 */
ISR_FUN hard_fault_interrupt(void)
{
    /* Assert the system. */
    OS_ASSERT(TRUE);

} /* hard_fault_interrupt */
