/*
 * stm32f411.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com>
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

#ifdef CONFIG_SLEEP
/* Used to manage 64 bit system clock. */
uint32_t clock_64_high_32 = 0;

/*
 * system_tick_Init
 * This is responsible for initializing system timer tick. This will not
 * enable the interrupt as it will be enabled after first context switch.
 */
void system_tick_Init(void)
{
    /* Configure system tick. */
    SysTick_Config((SYS_FREQ/SOFT_TICKS_PER_SEC));

    /* Configure TIMER2 for 64-bit timer. */

    /* Enable clock for TIMER1 */
    RCC->APB1ENR |= 0x1;

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
 * This is 64-bit clock interrupt handler.
 */
ISR_FUN isr_clock64_tick(void)
{
    ISR_ENTER();

    /* Clear the interrupt. */
    TIM2->SR = ~TIM_DIER_UIE;

    /* Timer roll over. */
    clock_64_high_32++;

    ISR_EXIT();

} /* isr_clock64_tick */

/*
 * current_hardware_tick
 * This returns value of a 64-bit hardware timer running at half of peripheral
 * frequency.
 */
uint64_t current_hardware_tick(void)
{
    /* Add and return current timer value. */
    return (((uint64_t)(clock_64_high_32 + ((TIM2->SR & TIM_DIER_UIE) ? 1 : 0)) * 0xFFFFFFFF) + (uint64_t)(TIM2->CNT));

} /* pit_get_clock */
#endif /* CONFIG_SLEEP */
