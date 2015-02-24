/*
 * isr.c
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

/* Initial vector table definition. */
__attribute__ ((section (".interrupts"))) VECTOR_TABLE system_isr_table =
{
    .callback =
    {
        (isr)&sys_stack_start,      /* -0x10  Top of Stack          */
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
        (isr)&cpu_interrupt,        /*  0x08  EXTI Line2            */
        (isr)&cpu_interrupt,        /*  0x09  EXTI Line3            */
        (isr)&cpu_interrupt,        /*  0x0A  EXTI Line4            */
        (isr)&cpu_interrupt,        /*  0x0B  DMA1 Stream 0         */
        (isr)&cpu_interrupt,        /*  0x0C  DMA1 Stream 1         */
        (isr)&cpu_interrupt,        /*  0x0D  DMA1 Stream 2         */
        (isr)&cpu_interrupt,        /*  0x0E  DMA1 Stream 3         */
        (isr)&cpu_interrupt,        /*  0x0F  DMA1 Stream 4         */
        (isr)&cpu_interrupt,        /*  0x10  DMA1 Stream 5         */
        (isr)&cpu_interrupt,        /*  0x11  DMA1 Stream 6         */
        (isr)&cpu_interrupt,        /*  0x12  ADC1, ADC2 and ADC3s  */
        (isr)&cpu_interrupt,        /*  0x13  CAN1 TX               */
        (isr)&cpu_interrupt,        /*  0x14  CAN1 RX0              */
        (isr)&cpu_interrupt,        /*  0x15  CAN1 RX1              */
        (isr)&cpu_interrupt,        /*  0x16  CAN1 SCE              */
        (isr)&cpu_interrupt,        /*  0x17  External Line[9:5]s   */
        (isr)&cpu_interrupt,        /*  0x18  TIM1 Break and TIM9       */
        (isr)&cpu_interrupt,        /*  0x19  TIM1 Update and TIM10     */
        (isr)&cpu_interrupt,        /*  0x1A  TIM1 Trigger and Commutation and TIM1     */
        (isr)&cpu_interrupt,        /*  0x1B  TIM1 Capture Compare      */
        (isr)&isr_clock64_tick,     /*  0x1C  TIM2                  */
        (isr)&cpu_interrupt,        /*  0x1D  TIM3                  */
        (isr)&cpu_interrupt,        /*  0x1E  TIM4                  */
        (isr)&cpu_interrupt,        /*  0x1F  I2C1 Event            */
        (isr)&cpu_interrupt,        /*  0x20  I2C1 Error            */
        (isr)&cpu_interrupt,        /*  0x21  I2C2 Event            */
        (isr)&cpu_interrupt,        /*  0x22  I2C2 Error            */
        (isr)&cpu_interrupt,        /*  0x23  SPI1                  */
        (isr)&cpu_interrupt,        /*  0x24  SPI2                  */
        (isr)&cpu_interrupt,        /*  0x25  USART1                */
        (isr)&cpu_interrupt,        /*  0x26  USART2                */
        (isr)&cpu_interrupt,        /*  0x27  USART3                */
        (isr)&cpu_interrupt,        /*  0x28  External Line[15:10]s     */
        (isr)&cpu_interrupt,        /*  0x29  RTC Alarm (A and B) through EXTI Line     */
        (isr)&cpu_interrupt,        /*  0x2A  USB OTG FS Wakeup through EXTI line       */
        (isr)&cpu_interrupt,        /*  0x2B  TIM8 Break and TIM12      */
        (isr)&cpu_interrupt,        /*  0x2C  TIM8 Update and TIM13     */
        (isr)&cpu_interrupt,        /*  0x2D  TIM8 Trigger and Commutation and TIM14    */
        (isr)&cpu_interrupt,        /*  0x2E  TIM8 Capture Compare      */
        (isr)&cpu_interrupt,        /*  0x2F  DMA1 Stream7          */
        (isr)&cpu_interrupt,        /*  0x30  FSMC                  */
        (isr)&cpu_interrupt,        /*  0x31  SDIO                  */
        (isr)&cpu_interrupt,        /*  0x32  TIM5                  */
        (isr)&cpu_interrupt,        /*  0x33  SPI3                  */
        (isr)&cpu_interrupt,        /*  0x34  UART4                 */
        (isr)&cpu_interrupt,        /*  0x35  UART5                 */
        (isr)&cpu_interrupt,        /*  0x36  TIM6 and DAC1&2 underrun errors           */
        (isr)&cpu_interrupt,        /*  0x37  TIM7                  */
        (isr)&cpu_interrupt,        /*  0x38  DMA2 Stream 0         */
        (isr)&cpu_interrupt,        /*  0x39  DMA2 Stream 1         */
        (isr)&cpu_interrupt,        /*  0x3A  DMA2 Stream 2         */
        (isr)&cpu_interrupt,        /*  0x3B  DMA2 Stream 3         */
        (isr)&cpu_interrupt,        /*  0x3C  DMA2 Stream 4         */
        (isr)&cpu_interrupt,        /*  0x3D  Ethernet              */
        (isr)&cpu_interrupt,        /*  0x3E  Ethernet Wakeup through EXTI line         */
        (isr)&cpu_interrupt,        /*  0x3F  CAN2 TX               */
        (isr)&cpu_interrupt,        /*  0x40  CAN2 RX0              */
        (isr)&cpu_interrupt,        /*  0x41  CAN2 RX1              */
        (isr)&cpu_interrupt,        /*  0x42  CAN2 SCE              */
#ifdef CONFIG_USB
        (isr)&usb_otg_interrupt,    /*  0x43  USB OTG FS            */
#else
        (isr)&cpu_interrupt,        /*  0x43  USB OTG FS            */
#endif
        (isr)&cpu_interrupt,        /*  0x44  DMA2 Stream 5         */
        (isr)&cpu_interrupt,        /*  0x45  DMA2 Stream 6         */
        (isr)&cpu_interrupt,        /*  0x46  DMA2 Stream 7         */
        (isr)&cpu_interrupt,        /*  0x47  USART6                */
        (isr)&cpu_interrupt,        /*  0x48  I2C3 event            */
        (isr)&cpu_interrupt,        /*  0x49  I2C3 error            */
        (isr)&cpu_interrupt,        /*  0x4A  USB OTG HS End Point 1 Out        */
        (isr)&cpu_interrupt,        /*  0x4B  USB OTG HS End Point 1 In         */
        (isr)&cpu_interrupt,        /*  0x4C  USB OTG HS Wakeup through EXTI    */
        (isr)&cpu_interrupt,        /*  0x4D  USB OTG HS            */
        (isr)&cpu_interrupt,        /*  0x4E  DCMI                  */
        (isr)&cpu_interrupt,        /*  0x4F  CRYP crypto           */
        (isr)&cpu_interrupt,        /*  0x50  Hash and Rng          */
        (isr)&cpu_interrupt,        /*  0x51  FPU                   */
    }
}; /* system_isr_table */
