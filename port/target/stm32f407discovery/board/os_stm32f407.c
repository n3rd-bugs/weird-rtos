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

    /* Configure TIMER2 for 64-bit timer. */

    /* Enable clock for TIMER1 */
    RCC->APB1ENR |= 0x01;

    /* Count up, and count with the frequency of PCLK/2. */
    TIM2->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS | TIM_CR1_CKD);

    /* Reload when overflow. */
    TIM2->ARR = 0xFFFFFFFF;

    /* Clear the initial count. */
    TIM2->CNT = 0;

    /* No pre-scale. */
    TIM2->PSC = 0;

    /* Enable timer. */
    TIM2->CR1 |= TIM_CR1_CEN;

    /* Enable timer interrupt. */
    TIM2->DIER |=  TIM_DIER_UIE;

    /* Enable timer interrupt in NVIC. */
    NVIC->ISER[28 >> 5] |= (1 << 28);

} /* system_tick_Init */

/*
 * isr_servicecall_handle
 * This pendSV interrupt handle.
 */
ISR_FUN isr_clock64_tick(void)
{
    OS_ISR_ENTER();

    /* Clear the interrupt. */
    TIM2->SR = ~TIM_DIER_UIE;

    /* Timer roll over. */
    clock_64_high_32++;

    OS_ISR_EXIT();

} /* isr_clock64_tick */

/*
 * pit_get_clock
 * This returns value of a 64-bit hardware timer running at half of peripheral
 * frequency.
 * User should not use this clock for time keeping that require higher 32-bits.
 */
uint64_t pit_get_clock()
{
    /* Load the soft-clock value. */
    uint64_t c_val = (uint32_t)clock_64_high_32;

    /* Make it higher 32-bits. */
    c_val = (c_val << 31);

    /* Add and return current timer value. */
    return (c_val + (TIM2->CNT));

} /* pit_get_clock */
