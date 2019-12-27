/*
 * init.c
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
#include <isr.h>

/*
 * wdt_disbale
 * This function disables the WDT.
 */
void wdt_disbale(void)
{
    ;
} /* wdt_disbale */

/*
 * sysclock_init
 * This function initializes system clock.
 */
void sysclock_init(void)
{
    /* Set HSION bit. */
    RCC->CR |= (uint32_t)0x00000001;

    /* Reset CFGR register. */
    RCC->CFGR = 0x08FFB80C;

    /* Reset HSEON, CSSON and PLLON bits. */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Reset PLLSRC, PLLXTPRE and PLLMUL[3:0] bits. */
    RCC->CFGR &= (uint32_t)0xFFC0FFFF;

    /* Reset PREDIV1[3:0] bits */
    RCC->CFGR2 &= (uint32_t)0xFFFFFFF0;

    /* Reset USARTSW[1:0], I2CSW, CECSW and ADCSW bits. */
    RCC->CFGR3 &= (uint32_t)0xFFFFFEEC;

    /* Reset HSI14 bit. */
    RCC->CR2 &= (uint32_t)0xFFFFFFFE;

    /* Disable all interrupts. */
    RCC->CIR = 0x00000000;

    /* Configure Flash prefetch, Instruction cache, Data cache and wait state. */
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

    /* Enable HSE. */
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);
    while ((RCC->CR & RCC_CR_HSERDY) == 0);

    /* HCLK = SYSCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

    /* PCLK = HCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE_DIV1;

    /* PLL configuration = HSE * 6 = 48 MHz. */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLMULL6);

    /* Enable PLL. */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till PLL is ready. */
    while((RCC->CR & RCC_CR_PLLRDY) == 0);

    /* Select PLL as system clock source. */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

    /* Wait till PLL is used as system clock source. */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL);

}

/*
 * system_entry
 * This is system entry function, this will initialize the hardware and then
 * call the user initializer.
 */
NAKED_FUN system_entry(void)
{
#ifdef TASK_STATS
    register uint32_t sp asm ("sp");
    uint8_t *stack = (uint8_t *)&_ebss;
#endif /* TASK_STATS */

    /* Initialize system clock. */
    sysclock_init();

    /* Initialize BSS. */
    asm (
    "   movs    r1, #0          \r\n"
    "   b       BSS_INIT        \r\n"

    "INIT_STATIC:\r\n"
    "   ldr     r3, =_sidata    \r\n"
    "   ldr     r3, [r3, r1]    \r\n"
    "   str     r3, [r0, r1]    \r\n"
    "   add     r1, r1, #4      \r\n"

    "BSS_INIT:\r\n"
    "   ldr     r0, =_sdata     \r\n"
    "   ldr     r3, =_edata     \r\n"
    "   add     r2, r0, r1      \r\n"
    "   cmp     r2, r3          \r\n"
    "   bcc     INIT_STATIC     \r\n"
    "   ldr     r2, =_sbss      \r\n"
    "   b       CLEAR_BSS       \r\n"

    "CLEAR_REGION:              \r\n"
    "   movs    r3, #0          \r\n"
    "   add     r2, #4          \r\n"
    "   str     r3, [r2]        \r\n"

    "CLEAR_BSS:                 \r\n"
    "   ldr     r3, = _ebss     \r\n"
    "   cmp     r2, r3          \r\n"
    "   bcc     CLEAR_REGION    \r\n"
    );

#ifdef TASK_STATS
    /* Load a predefined pattern on the system stack until we hit the
     * stack pointer. */
    while ((uint8_t *)sp > stack)
    {
        /* Load a predefined pattern. */
        *(stack++) = TASK_STACK_PATTERN;
    }
#endif /* TASK_STATS */

    /* Initialize IO. */
    rtl_arm_init();

    /* Disable watch dog timer. */
    wdt_disbale();

    /* We are not running any task until OS initializes. */
    set_current_task(NULL);

    /* Disable interrupts. */
    DISABLE_INTERRUPTS();

    /* Call application initializer. */
    (void) main();

    /* We should never return from this function. */
    for (;;) ;

} /* system_entry */
