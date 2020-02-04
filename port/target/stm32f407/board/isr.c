/*
 * isr.c
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
void __attribute__ ((weak, alias("cpu_interrupt"))) exti2_interrupt(void);
void __attribute__ ((weak, alias("cpu_interrupt"))) usart1_interrupt(void);

/* Initial vector table definition. */
__attribute__ ((section (".interrupts"), used)) ISR system_isr_table[] =
{
    (ISR)STM32F407_STACK_END,   /* -0x10  Top of Stack          */
    (ISR)&system_entry,         /* -0xF  Reset Handler          */
    (ISR)&nmi_interrupt,        /* -0xE  NMI Handler            */
    (ISR)&hard_fault_interrupt, /* -0xD  Hard Fault Handler     */
    (ISR)&cpu_interrupt,        /* -0xC  MPU Fault Handler      */
    (ISR)&cpu_interrupt,        /* -0xB  Bus Fault Handler      */
    (ISR)&cpu_interrupt,        /* -0xA  Usage Fault Handler    */
    (ISR)&cpu_interrupt,        /* -0x9                         */
    (ISR)&cpu_interrupt,        /* -0x8                         */
    (ISR)&cpu_interrupt,        /* -0x7                         */
    (ISR)&cpu_interrupt,        /* -0x6                         */
    (ISR)&isr_sv_handle,        /* -0x5  SVCall Handler         */
    (ISR)&cpu_interrupt,        /* -0x4  Debug Monitor Handler  */
    (ISR)&cpu_interrupt,        /* -0x3                         */
    (ISR)&isr_pendsv_handle,    /* -0x2  PendSV Handler         */
    (ISR)&isr_sysclock_handle,  /* -0x1  SysTick_Handler        */
    (ISR)&cpu_interrupt,        /*  0x0  Window WatchDog        */
    (ISR)&cpu_interrupt,        /*  0x1  PVD through EXTI Line detection                */
    (ISR)&cpu_interrupt,        /*  0x2  Tamper and TimeStamps through the EXTI line    */
    (ISR)&cpu_interrupt,        /*  0x3  RTC Wakeup through the EXTI line               */
    (ISR)&cpu_interrupt,        /*  0x4  FLASH                  */
    (ISR)&cpu_interrupt,        /*  0x5  RCC                    */
    (ISR)&cpu_interrupt,        /*  0x6  EXTI Line0             */
    (ISR)&cpu_interrupt,        /*  0x7  EXTI Line1             */
    (ISR)&exti2_interrupt,      /*  0x8  EXTI Line2             */
    (ISR)&cpu_interrupt,        /*  0x9  EXTI Line3             */
    (ISR)&cpu_interrupt,        /*  0xA  EXTI Line4             */
    (ISR)&cpu_interrupt,        /*  0xB  DMA1 Stream 0          */
    (ISR)&cpu_interrupt,        /*  0xC  DMA1 Stream 1          */
    (ISR)&cpu_interrupt,        /*  0xD  DMA1 Stream 2          */
    (ISR)&cpu_interrupt,        /*  0xE  DMA1 Stream 3          */
    (ISR)&cpu_interrupt,        /*  0xF  DMA1 Stream 4          */
    (ISR)&cpu_interrupt,        /*  0x10  DMA1 Stream 5         */
    (ISR)&cpu_interrupt,        /*  0x11  DMA1 Stream 6         */
    (ISR)&cpu_interrupt,        /*  0x12  ADC1, ADC2 and ADC3s  */
    (ISR)&cpu_interrupt,        /*  0x13  CAN1 TX               */
    (ISR)&cpu_interrupt,        /*  0x14  CAN1 RX0              */
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
    (ISR)&cpu_interrupt,        /*  0x1F  I2C1 Event            */
    (ISR)&cpu_interrupt,        /*  0x20  I2C1 Error            */
    (ISR)&cpu_interrupt,        /*  0x21  I2C2 Event            */
    (ISR)&cpu_interrupt,        /*  0x22  I2C2 Error            */
    (ISR)&cpu_interrupt,        /*  0x23  SPI1                  */
    (ISR)&cpu_interrupt,        /*  0x24  SPI2                  */
    (ISR)&usart1_interrupt,     /*  0x25  USART1                */
    (ISR)&cpu_interrupt,        /*  0x26  USART2                */
    (ISR)&cpu_interrupt,        /*  0x27  USART3                */
    (ISR)&cpu_interrupt,        /*  0x28  External Line[15:10]s */
    (ISR)&cpu_interrupt,        /*  0x29  RTC Alarm (A and B) through EXTI Line */
    (ISR)&cpu_interrupt,        /*  0x2A  USB OTG FS Wakeup through EXTI line   */
    (ISR)&cpu_interrupt,        /*  0x2B  TIM8 Break and TIM12  */
    (ISR)&cpu_interrupt,        /*  0x2C  TIM8 Update and TIM13 */
    (ISR)&cpu_interrupt,        /*  0x2D  TIM8 Trigger and Commutation and TIM14    */
    (ISR)&cpu_interrupt,        /*  0x2E  TIM8 Capture Compare  */
    (ISR)&cpu_interrupt,        /*  0x2F  DMA1 Stream7          */
    (ISR)&cpu_interrupt,        /*  0x30  FSMC                  */
    (ISR)&cpu_interrupt,        /*  0x31  SDIO                  */
    (ISR)&cpu_interrupt,        /*  0x32  TIM5                  */
    (ISR)&cpu_interrupt,        /*  0x33  SPI3                  */
    (ISR)&cpu_interrupt,        /*  0x34  UART4                 */
    (ISR)&cpu_interrupt,        /*  0x35  UART5                 */
    (ISR)&cpu_interrupt,        /*  0x36  TIM6 and DAC1&2 underrun errors   */
    (ISR)&cpu_interrupt,        /*  0x37  TIM7                  */
    (ISR)&cpu_interrupt,        /*  0x38  DMA2 Stream 0         */
    (ISR)&cpu_interrupt,        /*  0x39  DMA2 Stream 1         */
    (ISR)&cpu_interrupt,        /*  0x3A  DMA2 Stream 2         */
    (ISR)&cpu_interrupt,        /*  0x3B  DMA2 Stream 3         */
    (ISR)&cpu_interrupt,        /*  0x3C  DMA2 Stream 4         */
    (ISR)&cpu_interrupt,        /*  0x3D  Ethernet              */
    (ISR)&cpu_interrupt,        /*  0x3E  Ethernet Wakeup through EXTI line */
    (ISR)&cpu_interrupt,        /*  0x3F  CAN2 TX               */
    (ISR)&cpu_interrupt,        /*  0x40  CAN2 RX0              */
    (ISR)&cpu_interrupt,        /*  0x41  CAN2 RX1              */
    (ISR)&cpu_interrupt,        /*  0x42  CAN2 SCE              */
    (ISR)&cpu_interrupt,        /*  0x43  USB OTG FS            */
    (ISR)&cpu_interrupt,        /*  0x44  DMA2 Stream 5         */
    (ISR)&cpu_interrupt,        /*  0x45  DMA2 Stream 6         */
    (ISR)&cpu_interrupt,        /*  0x46  DMA2 Stream 7         */
    (ISR)&cpu_interrupt,        /*  0x47  USART6                */
    (ISR)&cpu_interrupt,        /*  0x48  I2C3 event            */
    (ISR)&cpu_interrupt,        /*  0x49  I2C3 error            */
    (ISR)&cpu_interrupt,        /*  0x4A  USB OTG HS End Point 1 Out        */
    (ISR)&cpu_interrupt,        /*  0x4B  USB OTG HS End Point 1 In         */
    (ISR)&cpu_interrupt,        /*  0x4C  USB OTG HS Wakeup through EXTI    */
    (ISR)&cpu_interrupt,        /*  0x4D  USB OTG HS            */
    (ISR)&cpu_interrupt,        /*  0x4E  DCMI                  */
    (ISR)&cpu_interrupt,        /*  0x4F  CRYP crypto           */
    (ISR)&cpu_interrupt,        /*  0x50  Hash and Rng          */
    (ISR)&cpu_interrupt,        /*  0x51  FPU                   */
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
