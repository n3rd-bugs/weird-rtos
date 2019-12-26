/*
 * stm32f407.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

/* Used to manage 64 bit system clock. */
uint32_t clock_64_high_32 = 0;

#ifdef CONFIG_SLEEP
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
#endif /* CONFIG_SLEEP */

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
