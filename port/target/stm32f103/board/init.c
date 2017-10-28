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

    /* Reset the RCC clock configuration to the default reset state. */

    /* Set HSION bit. */
    RCC->CR |= (uint32_t)0x00000001;

    /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits. */
    RCC->CFGR = 0xF8FF0000;

    /* Reset HSEON, CSSON and PLLON bits. */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset HSEBYP bit. */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits. */
    RCC->CFGR &= (uint32_t)0xFF80FFFF;

    /* Disable all interrupts and clear pending bits. */
    RCC->CIR = 0x00000000;

    /* Enable HSE. */
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);

    /* Wait while HSE is not enabled. */
    while ((RCC->CR & RCC_CR_HSERDY) == 0)
    {
        ;
    }

    /* Enable Prefetch Buffer. */
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    /* Flash 2 wait state. */
    FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
    FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_2;

    /* HCLK = SYSCLK. */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

    /* PCLK2 = HCLK. */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;

    /* PCLK1 = HCLK. */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2;

    /*  PLL configuration: PLLCLK = HSE * 9 = 72 MHz. */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
                                        RCC_CFGR_PLLMULL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMULL9);

    /* Enable PLL. */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till PLL is ready. */
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
        ;
    }

    /* Select PLL as system clock source. */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

    /* Wait till PLL is used as system clock source. */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08)
    {
        ;
    }

} /* sysclock_init */

/*
 * system_entry
 * This is system entry function, this will initialize the hardware and then
 * call the user initializer.
 */
NAKED_FUN system_entry(void)
{
#ifdef CONFIG_TASK_STATS
    register uint32_t sp asm ("sp");
    uint8_t *stack = (uint8_t *)SYSTEM_STACK;
#endif /* CONFIG_TASK_STATS */

    /* Adjust system vector table pointer. */
    SCB->VTOR = (uint32_t)(&system_isr_table);

    /* Initialize BSS. */
    asm (
    "   movs    r1, #0          \r\n"
    "   b       BSS_INIT        \r\n"

    "INIT_STATIC:\r\n"
    "   ldr     r3, =_sidata    \r\n"
    "   ldr     r3, [r3, r1]    \r\n"
    "   str     r3, [r0, r1]    \r\n"
    "   adds    r1, r1, #4      \r\n"

    "BSS_INIT:\r\n"
    "   ldr     r0, =_sdata     \r\n"
    "   ldr     r3, =_edata     \r\n"
    "   adds    r2, r0, r1      \r\n"
    "   cmp     r2, r3          \r\n"
    "   bcc     INIT_STATIC     \r\n"
    "   ldr     r2, =_sbss      \r\n"
    "   b       CLEAR_BSS       \r\n"

    "CLEAR_REGION:              \r\n"
    "   movs    r3, #0          \r\n"
    "   str     r3, [r2], #4    \r\n"

    "CLEAR_BSS:                 \r\n"
    "   ldr     r3, = _ebss     \r\n"
    "   cmp     r2, r3          \r\n"
    "   bcc     CLEAR_REGION    \r\n"
    );

#ifdef CONFIG_TASK_STATS
    /* Load a predefined pattern on the system stack until we hit the
     * stack pointer. */
    while ((uint8_t *)sp > stack)
    {
        /* Load a predefined pattern. */
        *(stack++) = CONFIG_STACK_PATTERN;
    }
#endif /* CONFIG_TASK_STATS */

    /* Initialize IO. */
    io_arm_init();

    /* Initialize system clock. */
    sysclock_init();

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
