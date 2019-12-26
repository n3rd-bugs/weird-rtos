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
#ifdef CONFIG_SERIAL
#include <serial.h>
#include <rtl.h>
#endif /* CONFIG_SERIAL */

/* ISR definitions. */
void __attribute__ ((weak, alias("cpu_interrupt"))) nmi_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) hard_fault_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) isr_sysclock_handle(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) isr_clock64_tick(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) usart1_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) usart2_interrupt(void);

/* Initial vector table definition. */
__attribute__ ((section (".interrupts"), used)) ISR system_isr_table[] =
{
    (ISR)STM32F411_STACK_END,   /* -0x10  Top of Stack                                  */
    (ISR)&system_entry,         /* -0x0F  Reset Handler                                 */
    (ISR)&nmi_interrupt,        /* -0x0E  NMI Handler                                   */
    (ISR)&hard_fault_interrupt, /* -0x0D  Hard Fault Handler                            */
    (ISR)&cpu_interrupt,        /* -0x0C  MPU Fault Handler                             */
    (ISR)&cpu_interrupt,        /* -0x0B  Bus Fault Handler                             */
    (ISR)&cpu_interrupt,        /* -0x0A  Usage Fault Handler                           */
    (ISR)&cpu_interrupt,        /* -0x09                                                */
    (ISR)&cpu_interrupt,        /* -0x08                                                */
    (ISR)&cpu_interrupt,        /* -0x07                                                */
    (ISR)&cpu_interrupt,        /* -0x06                                                */
    (ISR)&isr_sv_handle,        /* -0x05  SVCall Handler                                */
    (ISR)&cpu_interrupt,        /* -0x04  Debug Monitor Handler                         */
    (ISR)&cpu_interrupt,        /* -0x03                                                */
    (ISR)&isr_pendsv_handle,    /* -0x02  PendSV Handler                                */
    (ISR)&isr_sysclock_handle,  /* -0x01  SysTick_Handler                               */
    (ISR)&cpu_interrupt,        /*  0x00  Window WatchDog                               */
    (ISR)&cpu_interrupt,        /*  0x01  PVD through EXTI Line detection               */
    (ISR)&cpu_interrupt,        /*  0x02  Tamper and TimeStamps through the EXTI line   */
    (ISR)&cpu_interrupt,        /*  0x03  RTC Wakeup through the EXTI line              */
    (ISR)&cpu_interrupt,        /*  0x04  FLASH                                         */
    (ISR)&cpu_interrupt,        /*  0x05  RCC                                           */
    (ISR)&cpu_interrupt,        /*  0x06  EXTI Line0                                    */
    (ISR)&cpu_interrupt,        /*  0x07  EXTI Line1                                    */
    (ISR)&cpu_interrupt,        /*  0x08  EXTI Line2                                    */
    (ISR)&cpu_interrupt,        /*  0x09  EXTI Line3                                    */
    (ISR)&cpu_interrupt,        /*  0x0a  EXTI Line4                                    */
    (ISR)&cpu_interrupt,        /*  0x0b  DMA1 Stream 0                                 */
    (ISR)&cpu_interrupt,        /*  0x0c  DMA1 Stream 1                                 */
    (ISR)&cpu_interrupt,        /*  0x0d  DMA1 Stream 2                                 */
    (ISR)&cpu_interrupt,        /*  0x0e  DMA1 Stream 3                                 */
    (ISR)&cpu_interrupt,        /*  0x0f  DMA1 Stream 4                                 */
    (ISR)&cpu_interrupt,        /*  0x10  DMA1 Stream 5                                 */
    (ISR)&cpu_interrupt,        /*  0x11  DMA1 Stream 6                                 */
    (ISR)&cpu_interrupt,        /*  0x12  ADC1, ADC2 and ADC3s                          */
    (ISR)&cpu_interrupt,        /*  0x13  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x14  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x15  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x16  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x17  External Line[9:5]s                           */
    (ISR)&cpu_interrupt,        /*  0x18  TIM1 Break and TIM9                           */
    (ISR)&cpu_interrupt,        /*  0x19  TIM1 Update and TIM10                         */
    (ISR)&cpu_interrupt,        /*  0x1a  TIM1 Trigger and Commutation and TIM11        */
    (ISR)&cpu_interrupt,        /*  0x1b  TIM1 Capture Compare                          */
    (ISR)&isr_clock64_tick,     /*  0x1c  TIM2                                          */
    (ISR)&cpu_interrupt,        /*  0x1d  TIM3                                          */
    (ISR)&cpu_interrupt,        /*  0x1e  TIM4                                          */
    (ISR)&cpu_interrupt,        /*  0x1f  I2C1 Event                                    */
    (ISR)&cpu_interrupt,        /*  0x20  I2C1 Error                                    */
    (ISR)&cpu_interrupt,        /*  0x21  I2C2 Event                                    */
    (ISR)&cpu_interrupt,        /*  0x22  I2C2 Error                                    */
    (ISR)&cpu_interrupt,        /*  0x23  SPI1                                          */
    (ISR)&cpu_interrupt,        /*  0x24  SPI2                                          */
    (ISR)&usart1_interrupt,     /*  0x25  USART1                                        */
    (ISR)&usart2_interrupt,     /*  0x26  USART2                                        */
    (ISR)&cpu_interrupt,        /*  0x27  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x28  External Line[15:10]s                         */
    (ISR)&cpu_interrupt,        /*  0x29  RTC Alarm (A and B) through EXTI Line         */
    (ISR)&cpu_interrupt,        /*  0x2a  USB OTG FS Wakeup through EXTI line           */
    (ISR)&cpu_interrupt,        /*  0x2b  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x2c  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x2d  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x2e  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x2f  DMA1 Stream7                                  */
    (ISR)&cpu_interrupt,        /*  0x30  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x31  SDIO                                          */
    (ISR)&cpu_interrupt,        /*  0x32  TIM5                                          */
    (ISR)&cpu_interrupt,        /*  0x33  SPI3                                          */
    (ISR)&cpu_interrupt,        /*  0x34  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x35  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x36  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x37  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x38  DMA2 Stream 0                                 */
    (ISR)&cpu_interrupt,        /*  0x39  DMA2 Stream 1                                 */
    (ISR)&cpu_interrupt,        /*  0x3a  DMA2 Stream 2                                 */
    (ISR)&cpu_interrupt,        /*  0x3b  DMA2 Stream 3                                 */
    (ISR)&cpu_interrupt,        /*  0x3c  DMA2 Stream 4                                 */
    (ISR)&cpu_interrupt,        /*  0x3d  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x3e  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x3f  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x40  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x41  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x42  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x43  USB OTG FS                                    */
    (ISR)&cpu_interrupt,        /*  0x44  DMA2 Stream 5                                 */
    (ISR)&cpu_interrupt,        /*  0x45  DMA2 Stream 6                                 */
    (ISR)&cpu_interrupt,        /*  0x46  DMA2 Stream 7                                 */
    (ISR)&cpu_interrupt,        /*  0x47  USART6                                        */
    (ISR)&cpu_interrupt,        /*  0x48  I2C3 event                                    */
    (ISR)&cpu_interrupt,        /*  0x49  I2C3 error                                    */
    (ISR)&cpu_interrupt,        /*  0x4a  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x4b  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x4c  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x4d  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x4e  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x4f  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x50  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x51  FPU                                           */
    (ISR)&cpu_interrupt,        /*  0x52  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x53  Reserved                                      */
    (ISR)&cpu_interrupt,        /*  0x54  SPI4                                          */
    (ISR)&cpu_interrupt,        /*  0x55  SPI5                                          */
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
