/*
 * os_pk40x256vlq100.c
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

uint32_t clock_64_high_32 = 0;

/*
 * isr_servicecall_handle
 * This pendSV interrupt handle.
 */
ISR_FUN isr_clock64_tick(void)
{
    OS_ISR_ENTER();

    /* Timer roll over. */
    clock_64_high_32++;

    /* Clear the interrupt flag. */
    PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
    PIT_TCTRL0 = (PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK);

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

    /* Make it higher word. */
    c_val = (c_val << 31);

    /* Load the hard-clock value. */
    c_val += (uint32_t)(0xFFFFFFFF - PIT_CVAL0);

    /* Return the  */
    return (c_val);

} /* pit_get_clock */

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
