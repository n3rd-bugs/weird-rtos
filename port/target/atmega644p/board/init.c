/*
 * init.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
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
#ifdef CONFIG_TASK_STATS
    uint8_t *stack = &__heap_start;

    /* Load a predefined pattern on the system stack until we hit the
     * stack pointer. */
    while ((uint8_t *)SP > stack)
    {
        /* Load a predefined pattern. */
        *(stack++) = CONFIG_STACK_PATTERN;
    }
#endif /* CONFIG_TASK_STATS */
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
    io_avr_init();

} /* avr_sys_stack_pointer_save */
