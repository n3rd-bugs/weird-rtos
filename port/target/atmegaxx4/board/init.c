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
 * avr_stack_init
 * This function will disable WDT and perform boatload operation if required.
 */
void avr_stack_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void avr_stack_init(void)
{
    /* Disable watch dog timer. */
    MCUSR = 0;
    wdt_disable();

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

} /* avr_stack_init */

/*
 * avr_sys_stack_pointer_save
 * This function will save the system stack pointer to be used when needed.
 */
void avr_sys_stack_pointer_save(void) __attribute__((naked)) __attribute__((section(".init8")));
void avr_sys_stack_pointer_save(void)
{
    /* Save the system stack pointer. */
    system_stack_end = (uint8_t *)SP;

    /* Hook STDIO. */
    rtl_avr_init();

} /* avr_sys_stack_pointer_save */
