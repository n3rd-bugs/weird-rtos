/*
 * init.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>
#include <avr/wdt.h>
#include <avr.h>

/*
 * avr_stack_fill
 * This function will fill the stack with predefined pattern.
 */
void avr_stack_fill(void) __attribute__((naked)) __attribute__((section(".init1")));
void avr_stack_fill(void)
{
#ifdef TASK_STATS
    uint8_t *stack = &__heap_start;

    /* Load a predefined pattern on the system stack until we hit the
     * stack pointer. */
    while ((uint8_t *)SP > stack)
    {
        /* Load a predefined pattern. */
        *(stack++) = TASK_STACK_PATTERN;
    }
#endif /* TASK_STATS */
} /* avr_stack_fill */

/*
 * avr_pre_clear
 * This function will perform pre-BSS clear operations including reseting
 * watchdog timer.
 */
void avr_pre_clear(void) __attribute__((naked)) __attribute__((section(".init3")));
void avr_pre_clear(void)
{
    /* Disable watch dog timer. */
    MCUSR = 0;
    wdt_disable();

} /* avr_pre_clear */

/*
 * avr_sys_setup
 * This function will perform initial system setup so RTOS can be initiated.
 */
void avr_sys_setup(void) __attribute__((naked)) __attribute__((section(".init8")));
void avr_sys_setup(void)
{
    /* Setup system stack. */
    SP = RAMEND;
    system_stack_end = SP;

#ifdef CONFIG_BOOTLOAD
    /* Initialize boot loader condition. */
    BOOTLOAD_COND_INIT;

    /* If boot condition meets. */
    if (BOOTLOAD_COND)
    {
        /* Perform boot load operation. */
        BOOTLOAD();
    }
#endif

    /* Hook STDIO. */
    rtl_avr_init();

    /* Disable system interrupts. */
    DISABLE_INTERRUPTS();

} /* avr_sys_stack_pointer_save */
