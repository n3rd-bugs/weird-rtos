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
#ifdef CONFIG_SERIAL
#include <serial.h>
#include <rtl.h>
#endif /* CONFIG_SERIAL */

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
    (ISR)&system_entry,                 /* -0x0F  Reset Handler                             */
    (ISR)&nmi_interrupt,                /* -0x0E  NMI Handler                               */
    (ISR)&hard_fault_interrupt,         /* -0x0D                                            */
    (ISR)&cpu_interrupt,                /* -0x0C                                            */
    (ISR)&cpu_interrupt,                /* -0x0B                                            */
    (ISR)&cpu_interrupt,                /* -0x0A                                            */
    (ISR)&cpu_interrupt,                /* -0x09                                            */
    (ISR)&cpu_interrupt,                /* -0x08                                            */
    (ISR)&cpu_interrupt,                /* -0x07                                            */
    (ISR)&cpu_interrupt,                /* -0x06                                            */
    (ISR)&isr_sv_handle,                /* -0x05  SVCall Handler                            */
    (ISR)&cpu_interrupt,                /* -0x04                                            */
    (ISR)&cpu_interrupt,                /* -0x03                                            */
    (ISR)&isr_pendsv_handle,            /* -0x02  PendSV Handler                            */
    (ISR)&isr_sysclock_handle,          /* -0x01  SysTick_Handler                           */
    (ISR)&cpu_interrupt,                /*  0x00  Window WatchDog                           */
    (ISR)&cpu_interrupt,                /*  0x01                                            */
    (ISR)&cpu_interrupt,                /*  0x02  RTC Wakeup through the EXTI line          */
    (ISR)&cpu_interrupt,                /*  0x03  FLASH                                     */
    (ISR)&cpu_interrupt,                /*  0x04  RCC                                       */
    (ISR)&cpu_interrupt,                /*  0x05  EXTI Line 0 - 1                           */
    (ISR)&cpu_interrupt,                /*  0x06  EXTI Line 2 - 3                           */
    (ISR)&cpu_interrupt,                /*  0x07  EXTI Line 4 - 15                          */
    (ISR)&cpu_interrupt,                /*  0x08                                            */
    (ISR)&cpu_interrupt,                /*  0x09  DMA1 Stream 1                             */
    (ISR)&cpu_interrupt,                /*  0x0a  DMA1 Stream 2 - 3                         */
    (ISR)&cpu_interrupt,                /*  0x0b  DMA1 Stream 4 - 5                         */
    (ISR)&cpu_interrupt,                /*  0x0c  ADC1                                      */
    (ISR)&cpu_interrupt,                /*  0x0d  TIM1 Break Update Trigger and Commutation */
    (ISR)&cpu_interrupt,                /*  0x0e  TIM1 Capture Compare                      */
    (ISR)&cpu_interrupt,                /*  0x0f                                            */
    (ISR)&cpu_interrupt,                /*  0x10  TIM3                                      */
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
