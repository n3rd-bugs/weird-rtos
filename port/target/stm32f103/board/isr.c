/*
 * isr.c
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
#include <isr.h>
#include <kernel.h>
#ifdef CONFIG_SERIAL
#include <serial.h>
#include <rtl.h>
#endif /* CONFIG_SERIAL */

/* ISR definitions. */
void __attribute__ ((weak, alias("cpu_interrupt"))) nmi_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) hard_fault_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) isr_sysclock_handle(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) isr_clock64_tick(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) exti15_10_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) i2c1_event_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) i2c1_error_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) usart1_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) usart2_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) usart3_interrupt(void);

/* Initial vector table definition. */
__attribute__ ((section (".interrupts"), used)) ISR system_isr_table[] =
{
    (ISR)STM32F103_STACK_END,   /* -0x10  Top of Stack          */
    (ISR)&system_entry,         /* -0x0F  Reset Handler         */
    (ISR)&nmi_interrupt,        /* -0x0E  NMI Handler           */
    (ISR)&hard_fault_interrupt, /* -0x0D  Hard Fault Handler    */
    (ISR)&cpu_interrupt,        /* -0x0C  MPU Fault Handler     */
    (ISR)&cpu_interrupt,        /* -0x0B  Bus Fault Handler     */
    (ISR)&cpu_interrupt,        /* -0x0A  Usage Fault Handler   */
    (ISR)&cpu_interrupt,        /* -0x09                        */
    (ISR)&cpu_interrupt,        /* -0x08                        */
    (ISR)&cpu_interrupt,        /* -0x07                        */
    (ISR)&cpu_interrupt,        /* -0x06                        */
    (ISR)&isr_sv_handle,        /* -0x05  SVCall Handler        */
    (ISR)&cpu_interrupt,        /* -0x04  Debug Monitor Handler */
    (ISR)&cpu_interrupt,        /* -0x03                        */
    (ISR)&isr_pendsv_handle,    /* -0x02  PendSV Handler        */
    (ISR)&isr_sysclock_handle,  /* -0x01  SysTick_Handler       */
    (ISR)&cpu_interrupt,        /*  0x00  Window WatchDog       */
    (ISR)&cpu_interrupt,        /*  0x01  PVD through EXTI Line detection               */
    (ISR)&cpu_interrupt,        /*  0x02  Tamper and TimeStamps through the EXTI line   */
    (ISR)&cpu_interrupt,        /*  0x03  RTC Wakeup through the EXTI line              */
    (ISR)&cpu_interrupt,        /*  0x04  FLASH                 */
    (ISR)&cpu_interrupt,        /*  0x05  RCC                   */
    (ISR)&cpu_interrupt,        /*  0x06  EXTI Line0            */
    (ISR)&cpu_interrupt,        /*  0x07  EXTI Line1            */
    (ISR)&cpu_interrupt,        /*  0x08  EXTI Line2            */
    (ISR)&cpu_interrupt,        /*  0x09  EXTI Line3            */
    (ISR)&cpu_interrupt,        /*  0x0A  EXTI Line4            */
    (ISR)&cpu_interrupt,        /*  0x0B  DMA1 Stream 0         */
    (ISR)&cpu_interrupt,        /*  0x0C  DMA1 Stream 1         */
    (ISR)&cpu_interrupt,        /*  0x0D  DMA1 Stream 2         */
    (ISR)&cpu_interrupt,        /*  0x0E  DMA1 Stream 3         */
    (ISR)&cpu_interrupt,        /*  0x0F  DMA1 Stream 4         */
    (ISR)&cpu_interrupt,        /*  0x10  DMA1 Stream 5         */
    (ISR)&cpu_interrupt,        /*  0x11  DMA1 Stream 6         */
    (ISR)&cpu_interrupt,        /*  0x12  ADCs                  */
    (ISR)&cpu_interrupt,        /*  0x13  USB HP / CAN1 TX      */
    (ISR)&cpu_interrupt,        /*  0x14  USB LP / CAN1 RX0     */
    (ISR)&cpu_interrupt,        /*  0x15  CAN1 RX1              */
    (ISR)&cpu_interrupt,        /*  0x16  CAN1 SCE              */
    (ISR)&cpu_interrupt,        /*  0x17  External Line[9:5]s   */
    (ISR)&cpu_interrupt,        /*  0x18  TIM1 Break and TIM9   */
    (ISR)&cpu_interrupt,        /*  0x19  TIM1 Update and TIM10 */
    (ISR)&cpu_interrupt,        /*  0x1A  TIM1 Trigger and Commutation and TIM1 */
    (ISR)&cpu_interrupt,        /*  0x1B  TIM1 Capture Compare  */
    (ISR)&isr_clock64_tick,     /*  0x1C  TIM2                  */
    (ISR)&cpu_interrupt,        /*  0x1D  TIM3                  */
    (ISR)&cpu_interrupt,        /*  0x1E  TIM4                  */
    (ISR)&i2c1_event_interrupt, /*  0x1F  I2C1 Event            */
    (ISR)&i2c1_error_interrupt, /*  0x20  I2C1 Error            */
    (ISR)&cpu_interrupt,        /*  0x21  I2C2 Event            */
    (ISR)&cpu_interrupt,        /*  0x22  I2C2 Error            */
    (ISR)&cpu_interrupt,        /*  0x23  SPI1                  */
    (ISR)&cpu_interrupt,        /*  0x24  SPI2                  */
    (ISR)&usart1_interrupt,     /*  0x25  USART1                */
    (ISR)&usart2_interrupt,     /*  0x26  USART2                */
    (ISR)&usart3_interrupt,     /*  0x27  USART3                */
    (ISR)&exti15_10_interrupt,  /*  0x28  External Line[15:10]s */
    (ISR)&cpu_interrupt,        /*  0x29  RTC Alarm (A and B) through EXTI Line */
    (ISR)&cpu_interrupt,        /*  0x2A  USB OTG FS Wakeup through EXTI line   */
    (ISR)&cpu_interrupt,        /*  0x2B                        */
    (ISR)&cpu_interrupt,        /*  0x2C                        */
    (ISR)&cpu_interrupt,        /*  0x2D                        */
    (ISR)&cpu_interrupt,        /*  0x2E                        */
    (ISR)&cpu_interrupt,        /*  0x2F                        */
    (ISR)&cpu_interrupt,        /*  0x30                        */
    (ISR)&cpu_interrupt,        /*  0x31                        */
    (ISR)0xF108F85F,            /*  0x32  This is for boot in RAM mode  */
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
