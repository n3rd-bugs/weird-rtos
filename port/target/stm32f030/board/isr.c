/*
 * isr.c
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
#include <stdio.h>
#ifdef IO_SERIAL
#include <serial.h>
#include <rtl.h>
#endif /* IO_SERIAL */

/* ISR definitions. */
void __attribute__ ((weak, alias("cpu_interrupt"))) nmi_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) hard_fault_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) isr_sysclock_handle(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) i2c1_event_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) usart1_interrupt(void);

/* Initial vector table definition. */
__attribute__ ((section (".interrupts"), used)) ISR system_isr_table[] =
{
    (ISR)STM32F030_STACK_END,           /* -0x10  Top of Stack                              */
    (ISR)&system_entry,                 /* -0xF  Reset Handler                              */
    (ISR)&nmi_interrupt,                /* -0xE  NMI Handler                                */
    (ISR)&hard_fault_interrupt,         /* -0xD                                             */
    (ISR)&cpu_interrupt,                /* -0xC                                             */
    (ISR)&cpu_interrupt,                /* -0xB                                             */
    (ISR)&cpu_interrupt,                /* -0xA                                             */
    (ISR)&cpu_interrupt,                /* -0x9                                             */
    (ISR)&cpu_interrupt,                /* -0x8                                             */
    (ISR)&cpu_interrupt,                /* -0x7                                             */
    (ISR)&cpu_interrupt,                /* -0x6                                             */
    (ISR)&isr_sv_handle,                /* -0x5  SVCall Handler                             */
    (ISR)&cpu_interrupt,                /* -0x4                                             */
    (ISR)&cpu_interrupt,                /* -0x3                                             */
    (ISR)&isr_pendsv_handle,            /* -0x2  PendSV Handler                             */
    (ISR)&isr_sysclock_handle,          /* -0x1  SysTick_Handler                            */
    (ISR)&cpu_interrupt,                /*  0x0  Window WatchDog                            */
    (ISR)&cpu_interrupt,                /*  0x1                                             */
    (ISR)&cpu_interrupt,                /*  0x2  RTC Wakeup through the EXTI line           */
    (ISR)&cpu_interrupt,                /*  0x3  FLASH                                      */
    (ISR)&cpu_interrupt,                /*  0x4  RCC                                        */
    (ISR)&cpu_interrupt,                /*  0x5  EXTI Line 0 - 1                            */
    (ISR)&cpu_interrupt,                /*  0x6  EXTI Line 2 - 3                            */
    (ISR)&cpu_interrupt,                /*  0x7  EXTI Line 4 - 15                           */
    (ISR)&cpu_interrupt,                /*  0x8                                             */
    (ISR)&cpu_interrupt,                /*  0x9  DMA1 Stream 1                              */
    (ISR)&cpu_interrupt,                /*  0xa  DMA1 Stream 2 - 3                          */
    (ISR)&cpu_interrupt,                /*  0xb  DMA1 Stream 4 - 5                          */
    (ISR)&cpu_interrupt,                /*  0xc  ADC1                                       */
    (ISR)&cpu_interrupt,                /*  0xd  TIM1 Break Update Trigger and Computation  */
    (ISR)&cpu_interrupt,                /*  0xe  TIM1 Capture Compare                       */
    (ISR)&cpu_interrupt,                /*  0xf                                             */
    (ISR)&isr_clock64_tick,             /*  0x10  TIM3                                      */
    (ISR)&cpu_interrupt,                /*  0x11                                            */
    (ISR)&cpu_interrupt,                /*  0x12                                            */
    (ISR)&cpu_interrupt,                /*  0x13  TIM14                                     */
    (ISR)&cpu_interrupt,                /*  0x14  TIM15                                     */
    (ISR)&cpu_interrupt,                /*  0x15  TIM16                                     */
    (ISR)&cpu_interrupt,                /*  0x16  TIM17                                     */
    (ISR)&i2c1_event_interrupt,         /*  0x17  I2C1 Event                                */
    (ISR)&cpu_interrupt,                /*  0x18  I2C2 Event                                */
    (ISR)&cpu_interrupt,                /*  0x19  SPI1 Event                                */
    (ISR)&cpu_interrupt,                /*  0x1a  SPI2 Event                                */
    (ISR)&usart1_interrupt,             /*  0x1b  USART1                                    */
    (ISR)&cpu_interrupt,                /*  0x1c                                            */
    (ISR)&cpu_interrupt,                /*  0x1d                                            */
    (ISR)&cpu_interrupt,                /*  0x1e                                            */
    (ISR)&cpu_interrupt,                /*  0x1f                                            */
    (ISR)0xF108F85F                     /*  0x20                                            */
}; /* system_isr_table */

/*
 * cpu_interrupt
 * Default ISR callback.
 */
ISR_FUN cpu_interrupt(void)
{
#ifdef ASSERT_ENABLE
    uint8_t str[16];
#endif /* ASSERT_ENABLE */
    volatile uint32_t ipsr;

    /* Read IPSR to get the interrupt number. */
    asm("MRS %0, ipsr" : "=r"(ipsr));

#ifdef ASSERT_ENABLE
    /* Print IPSR. */
    serial_assert_puts((uint8_t *)"ISPR: ", 0);
    rtl_ultoa_b10(ipsr, str);
    serial_assert_puts(str, 0);
    serial_assert_puts((uint8_t *)"\r\n", 0);
#endif /* ASSERT_ENABLE */

    /* Assert the system. */
    ASSERT(TRUE);

} /* cpu_interrupt */
