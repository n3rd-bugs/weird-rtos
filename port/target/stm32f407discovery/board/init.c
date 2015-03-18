/*
 * init.c
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
#include <isr.h>

/*
 * wdt_disbale
 * This function disables the WDT.
 */
void wdt_disbale()
{
    ;
} /* wdt_disbale */

/*
 * sysclock_init
 * This function initializes system clock.
 */
void sysclock_init()
{

    /* Reset the RCC clock configuration to the default reset state. */

    /* Set HSION bit. */
    RCC->CR |= (uint32_t)0x00000001;

    /* Reset CFGR register. */
    RCC->CFGR = 0x00000000;

    /* Reset HSEON, CSSON and PLLON bits. */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset PLLCFGR register. */
    RCC->PLLCFGR = 0x24003010;

    /* Reset HSEBYP bit. */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Disable all interrupts. */
    RCC->CIR = 0x00000000;

    /* Enable HSE. */
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);

    /* Wait while HSE is not enabled. */
    while ((RCC->CR & RCC_CR_HSERDY) == 0)
    {
        ;
    }

    /* Enable high performance mode, System frequency up to 168 MHz. */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_PMODE;

    /* HCLK = SYSCLK / 1. */
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

    /* PCLK1 = HCLK / 2. */
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    /* PCLK2 = HCLK / 2. */
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

    /* Configure the main PLL. */
    /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N. */
    RCC->PLLCFGR = (8) | ((336) << 6);

    /* SYSCLK = PLL_VCO / PLL_P. */
    RCC->PLLCFGR |= ((((2) >> 1) -1) << 16) | (RCC_PLLCFGR_PLLSRC_HSE);

    /* USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ. */
    RCC->PLLCFGR |= ((7) << 24);

    /* Enable the main PLL. */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till the main PLL is ready. */
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
        ;
    }

    /* Configure Flash prefetch, Instruction cache, Data cache and wait state. */
    FLASH->ACR = FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;

    /* Select the main PLL as system clock source. */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    /* Wait till the main PLL is used as system clock source. */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);
    {
        ;
    }

} /* sysclock_init */

/*
 * system_entry
 * This is system entry function, this will initialize the hardware and then
 * call the user initializer.
 */
void system_entry(void)
{
    extern uint64_t current_tick;

    /* Adjust system vector table pointer. */
    SCB->VTOR = (uint32_t)(&system_isr_table);

    __asm (
    "   MOV    R0, %[sp]    \r\n"
    "   CMP    R0, #0       \r\n"
    "   BEQ    skip_sp      \r\n"
    "   mov    SP, R0       \r\n"
    "   sub    SP, #4       \r\n"
    "   mov    R0, #0       \r\n"
    "   mvn    R0, R0       \r\n"
    "   str    R0, [SP,#0]  \r\n"
    "   add    SP, #4       \r\n"
    " skip_sp:              \r\n"
    ::
    [sp] "r" (&sys_stack_start));

#if (CORTEX_M4_FPU == TRUE)
    /* Enable FPU co-processor. */
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
#endif

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

    /* Initialize system clock. */
    sysclock_init();

    /* Disable watch dog timer. */
    wdt_disbale();

    /* Initialize system clock. */
    current_tick = 0;

    /* We are not running any task until OS initializes. */
    set_current_task(NULL);

    ENABLE_INTERRUPTS();

    /* Call application initializer. */
    (void) main();

} /* system_entry */
