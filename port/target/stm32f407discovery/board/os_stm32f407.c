/*
 * os_stm32f407.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>

/* Used to manage 64 bit system clock. */
uint32_t clock_64_high_32 = 0;

/*
 * system_tick_Init
 * This is responsible for initializing system timer tick. This will not
 * enable the interrupt as it will be enabled after first context switch.
 */
void system_tick_Init()
{
    /* Configure system tick. */
    SysTick_Config((SYS_FREQ/OS_TICKS_PER_SEC));

} /* system_tick_Init */

/*
 * isr_servicecall_handle
 * This pendSV interrupt handle.
 */
ISR_FUN isr_clock64_tick(void)
{
    OS_ISR_ENTER();

    /* Timer roll over. */
    clock_64_high_32++;

    OS_ISR_EXIT();

} /* isr_clock64_tick */

/*
 * pit_get_clock
 * This returns value of a 64-bit hardware timer running at peripheral frequency.
 */
uint64_t pit_get_clock()
{
    /* Load the soft-clock value. */
    uint64_t c_val = (uint32_t)clock_64_high_32;

    /* Return the  */
    return (c_val);

} /* pit_get_clock */
