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

/* ISR definitions. */
void __attribute__ ((weak, alias("cpu_interrupt"))) nmi_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) hard_fault_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) exti2_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) i2c1_event_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) i2c1_error_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) usart1_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) usart2_interrupt(void);

/* Initial vector table definition. */
__attribute__ ((section (".interrupts"))) VECTOR_TABLE system_isr_table =
{
    .callback =
    {
        (isr)STM32F103_STACK_END,   /* -0x10  Top of Stack          */
        (isr)&system_entry,         /* -0x0F  Reset Handler         */
        (isr)&nmi_interrupt,        /* -0x0E  NMI Handler           */
        (isr)&hard_fault_interrupt, /* -0x0D  Hard Fault Handler    */
        (isr)&cpu_interrupt,        /* -0x0C  MPU Fault Handler     */
        (isr)&cpu_interrupt,        /* -0x0B  Bus Fault Handler     */
        (isr)&cpu_interrupt,        /* -0x0A  Usage Fault Handler   */
        (isr)&cpu_interrupt,        /* -0x09                        */
        (isr)&cpu_interrupt,        /* -0x08                        */
        (isr)&cpu_interrupt,        /* -0x07                        */
        (isr)&cpu_interrupt,        /* -0x06                        */
        (isr)&cpu_interrupt,        /* -0x05  SVCall Handler        */
        (isr)&cpu_interrupt,        /* -0x04  Debug Monitor Handler */
        (isr)&cpu_interrupt,        /* -0x03                        */
        (isr)&isr_pendsv_handle,    /* -0x02  PendSV Handler        */
        (isr)&isr_sysclock_handle,  /* -0x01  SysTick_Handler       */
        (isr)&cpu_interrupt,        /*  0x00  Window WatchDog       */
        (isr)&cpu_interrupt,        /*  0x01  PVD through EXTI Line detection               */
        (isr)&cpu_interrupt,        /*  0x02  Tamper and TimeStamps through the EXTI line   */
        (isr)&cpu_interrupt,        /*  0x03  RTC Wakeup through the EXTI line              */
        (isr)&cpu_interrupt,        /*  0x04  FLASH                 */
        (isr)&cpu_interrupt,        /*  0x05  RCC                   */
        (isr)&cpu_interrupt,        /*  0x06  EXTI Line0            */
        (isr)&cpu_interrupt,        /*  0x07  EXTI Line1            */
        (isr)&exti2_interrupt,      /*  0x08  EXTI Line2            */
        (isr)&cpu_interrupt,        /*  0x09  EXTI Line3            */
        (isr)&cpu_interrupt,        /*  0x0A  EXTI Line4            */
        (isr)&cpu_interrupt,        /*  0x0B  DMA1 Stream 0         */
        (isr)&cpu_interrupt,        /*  0x0C  DMA1 Stream 1         */
        (isr)&cpu_interrupt,        /*  0x0D  DMA1 Stream 2         */
        (isr)&cpu_interrupt,        /*  0x0E  DMA1 Stream 3         */
        (isr)&cpu_interrupt,        /*  0x0F  DMA1 Stream 4         */
        (isr)&cpu_interrupt,        /*  0x10  DMA1 Stream 5         */
        (isr)&cpu_interrupt,        /*  0x11  DMA1 Stream 6         */
        (isr)&cpu_interrupt,        /*  0x12  ADCs                  */
        (isr)&cpu_interrupt,        /*  0x13  USB HP / CAN1 TX      */
        (isr)&cpu_interrupt,        /*  0x14  USB LP / CAN1 RX0     */
        (isr)&cpu_interrupt,        /*  0x15  CAN1 RX1              */
        (isr)&cpu_interrupt,        /*  0x16  CAN1 SCE              */
        (isr)&cpu_interrupt,        /*  0x17  External Line[9:5]s   */
        (isr)&cpu_interrupt,        /*  0x18  TIM1 Break and TIM9   */
        (isr)&cpu_interrupt,        /*  0x19  TIM1 Update and TIM10 */
        (isr)&cpu_interrupt,        /*  0x1A  TIM1 Trigger and Commutation and TIM1 */
        (isr)&cpu_interrupt,        /*  0x1B  TIM1 Capture Compare  */
        (isr)&isr_clock64_tick,     /*  0x1C  TIM2                  */
        (isr)&cpu_interrupt,        /*  0x1D  TIM3                  */
        (isr)&cpu_interrupt,        /*  0x1E  TIM4                  */
        (isr)&i2c1_event_interrupt, /*  0x1F  I2C1 Event            */
        (isr)&i2c1_error_interrupt, /*  0x20  I2C1 Error            */
        (isr)&cpu_interrupt,        /*  0x21  I2C2 Event            */
        (isr)&cpu_interrupt,        /*  0x22  I2C2 Error            */
        (isr)&cpu_interrupt,        /*  0x23  SPI1                  */
        (isr)&cpu_interrupt,        /*  0x24  SPI2                  */
        (isr)&usart1_interrupt,     /*  0x25  USART1                */
        (isr)&usart2_interrupt,     /*  0x26  USART2                */
        (isr)&cpu_interrupt,        /*  0x27  USART3                */
        (isr)&cpu_interrupt,        /*  0x28  External Line[15:10]s */
        (isr)&cpu_interrupt,        /*  0x29  RTC Alarm (A and B) through EXTI Line */
        (isr)&cpu_interrupt,        /*  0x2A  USB OTG FS Wakeup through EXTI line   */
        (isr)&cpu_interrupt,        /*  0x2B                        */
        (isr)&cpu_interrupt,        /*  0x2C                        */
        (isr)&cpu_interrupt,        /*  0x2D                        */
        (isr)&cpu_interrupt,        /*  0x2E                        */
        (isr)&cpu_interrupt,        /*  0x2F                        */
        (isr)&cpu_interrupt,        /*  0x30                        */
        (isr)&cpu_interrupt,        /*  0x31                        */
        (isr)0xF108F85F,            /*  0x32  This is for boot in RAM mode  */
    }
}; /* system_isr_table */

/*
 * cpu_interrupt
 * Default ISR callback.
 */
ISR_FUN cpu_interrupt(void)
{
    /* Assert the system. */
    ASSERT(TRUE);

} /* cpu_interrupt */
