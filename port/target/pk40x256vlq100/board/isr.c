/*
 * isr.c
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
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

/* System entry function. */
extern void system_entry(void);

/* Initial vector table definition. */
__attribute__ ((section (".vectortable"))) VECTOR_TABLE system_isr_table =
{
    .callback =
    {
        /* ISR name                         No.   Address      Pri Name                     */
        (isr)&os_stack_end,         /*  0x00  0x00000000   -   INT_Initial_Stack_Pointer    */
        (isr)&system_entry,         /*  0x01  0x00000004   -   INT_Initial_Program_Counter  */
        (isr)&nmi_interrupt,        /*  0x02  0x00000008   -2  INT_NMI                      */
        (isr)&hard_fault_interrupt, /*  0x03  0x0000000C   -1  INT_Hard_Fault               */
        (isr)&cpu_interrupt,        /*  0x04  0x00000010   -   INT_Mem_Manage_Fault         */
        (isr)&cpu_interrupt,        /*  0x05  0x00000014   -   INT_Bus_Fault                */
        (isr)&cpu_interrupt,        /*  0x06  0x00000018   -   INT_Usage_Fault              */
        (isr)&cpu_interrupt,        /*  0x07  0x0000001C   -   INT_Reserved7                */
        (isr)&cpu_interrupt,        /*  0x08  0x00000020   -   INT_Reserved8                */
        (isr)&cpu_interrupt,        /*  0x09  0x00000024   -   INT_Reserved9                */
        (isr)&cpu_interrupt,        /*  0x0A  0x00000028   -   INT_Reserved10               */
        (isr)&cpu_interrupt,        /*  0x0B  0x0000002C   -   INT_SVCall                   */
        (isr)&cpu_interrupt,        /*  0x0C  0x00000030   -   INT_DebugMonitor             */
        (isr)&cpu_interrupt,        /*  0x0D  0x00000034   -   INT_Reserved13               */
        (isr)&isr_pendsv_handle,    /*  0x0E  0x00000038   -   INT_PendableSrvReq           */
        (isr)&isr_sysclock_handle,  /*  0x0F  0x0000003C   -   INT_SysTick                  */
        (isr)&cpu_interrupt,        /*  0x10  0x00000040   -   INT_DMA0                     */
        (isr)&cpu_interrupt,        /*  0x11  0x00000044   -   INT_DMA1                     */
        (isr)&cpu_interrupt,        /*  0x12  0x00000048   -   INT_DMA2                     */
        (isr)&cpu_interrupt,        /*  0x13  0x0000004C   -   INT_DMA3                     */
        (isr)&cpu_interrupt,        /*  0x14  0x00000050   -   INT_DMA4                     */
        (isr)&cpu_interrupt,        /*  0x15  0x00000054   -   INT_DMA5                     */
        (isr)&cpu_interrupt,        /*  0x16  0x00000058   -   INT_DMA6                     */
        (isr)&cpu_interrupt,        /*  0x17  0x0000005C   -   INT_DMA7                     */
        (isr)&cpu_interrupt,        /*  0x18  0x00000060   -   INT_DMA8                     */
        (isr)&cpu_interrupt,        /*  0x19  0x00000064   -   INT_DMA9                     */
        (isr)&cpu_interrupt,        /*  0x1A  0x00000068   -   INT_DMA10                    */
        (isr)&cpu_interrupt,        /*  0x1B  0x0000006C   -   INT_DMA11                    */
        (isr)&cpu_interrupt,        /*  0x1C  0x00000070   -   INT_DMA12                    */
        (isr)&cpu_interrupt,        /*  0x1D  0x00000074   -   INT_DMA13                    */
        (isr)&cpu_interrupt,        /*  0x1E  0x00000078   -   INT_DMA14                    */
        (isr)&cpu_interrupt,        /*  0x1F  0x0000007C   -   INT_DMA15                    */
        (isr)&cpu_interrupt,        /*  0x20  0x00000080   -   INT_DMA_Error                */
        (isr)&cpu_interrupt,        /*  0x21  0x00000084   -   INT_MCM                      */
        (isr)&cpu_interrupt,        /*  0x22  0x00000088   -   INT_FTFL                     */
        (isr)&cpu_interrupt,        /*  0x23  0x0000008C   -   INT_Read_Collision           */
        (isr)&cpu_interrupt,        /*  0x24  0x00000090   -   INT_LVD_LVW                  */
        (isr)&cpu_interrupt,        /*  0x25  0x00000094   -   INT_LLW                      */
        (isr)&cpu_interrupt,        /*  0x26  0x00000098   -   INT_Watchdog                 */
        (isr)&cpu_interrupt,        /*  0x27  0x0000009C   -   INT_Reserved39               */
        (isr)&cpu_interrupt,        /*  0x28  0x000000A0   -   INT_I2C0                     */
        (isr)&cpu_interrupt,        /*  0x29  0x000000A4   -   INT_I2C1                     */
        (isr)&cpu_interrupt,        /*  0x2A  0x000000A8   -   INT_SPI0                     */
        (isr)&cpu_interrupt,        /*  0x2B  0x000000AC   -   INT_SPI1                     */
        (isr)&cpu_interrupt,        /*  0x2C  0x000000B0   -   INT_SPI2                     */
        (isr)&cpu_interrupt,        /*  0x2D  0x000000B4   -   INT_CAN0_ORed_Message_buffer */
        (isr)&cpu_interrupt,        /*  0x2E  0x000000B8   -   INT_CAN0_Bus_Off             */
        (isr)&cpu_interrupt,        /*  0x2F  0x000000BC   -   INT_CAN0_Error               */
        (isr)&cpu_interrupt,        /*  0x30  0x000000C0   -   INT_CAN0_Tx_Warning          */
        (isr)&cpu_interrupt,        /*  0x31  0x000000C4   -   INT_CAN0_Rx_Warning          */
        (isr)&cpu_interrupt,        /*  0x32  0x000000C8   -   INT_CAN0_Wake_Up             */
        (isr)&cpu_interrupt,        /*  0x33  0x000000CC   -   INT_I2S0_Tx                  */
        (isr)&cpu_interrupt,        /*  0x34  0x000000D0   -   INT_I2S0_Rx                  */
        (isr)&cpu_interrupt,        /*  0x35  0x000000D4   -   INT_CAN1_ORed_Message_buffer */
        (isr)&cpu_interrupt,        /*  0x36  0x000000D8   -   INT_CAN1_Bus_Off             */
        (isr)&cpu_interrupt,        /*  0x37  0x000000DC   -   INT_CAN1_Error               */
        (isr)&cpu_interrupt,        /*  0x38  0x000000E0   -   INT_CAN1_Tx_Warning          */
        (isr)&cpu_interrupt,        /*  0x39  0x000000E4   -   INT_CAN1_Rx_Warning          */
        (isr)&cpu_interrupt,        /*  0x3A  0x000000E8   -   INT_CAN1_Wake_Up             */
        (isr)&cpu_interrupt,        /*  0x3B  0x000000EC   -   INT_Reserved59               */
        (isr)&cpu_interrupt,        /*  0x3C  0x000000F0   -   INT_UART0_LON                */
        (isr)&cpu_interrupt,        /*  0x3D  0x000000F4   -   INT_UART0_RX_TX              */
        (isr)&cpu_interrupt,        /*  0x3E  0x000000F8   -   INT_UART0_ERR                */
        (isr)&cpu_interrupt,        /*  0x3F  0x000000FC   -   INT_UART1_RX_TX              */
        (isr)&cpu_interrupt,        /*  0x40  0x00000100   -   INT_UART1_ERR                */
        (isr)&cpu_interrupt,        /*  0x41  0x00000104   -   INT_UART2_RX_TX              */
        (isr)&cpu_interrupt,        /*  0x42  0x00000108   -   INT_UART2_ERR                */
        (isr)&cpu_interrupt,        /*  0x43  0x0000010C   -   INT_UART3_RX_TX              */
        (isr)&cpu_interrupt,        /*  0x44  0x00000110   -   INT_UART3_ERR                */
        (isr)&cpu_interrupt,        /*  0x45  0x00000114   -   INT_UART4_RX_TX              */
        (isr)&cpu_interrupt,        /*  0x46  0x00000118   -   INT_UART4_ERR                */
        (isr)&cpu_interrupt,        /*  0x47  0x0000011C   -   INT_UART5_RX_TX              */
        (isr)&cpu_interrupt,        /*  0x48  0x00000120   -   INT_UART5_ERR                */
        (isr)&cpu_interrupt,        /*  0x49  0x00000124   -   INT_ADC0                     */
        (isr)&cpu_interrupt,        /*  0x4A  0x00000128   -   INT_ADC1                     */
        (isr)&cpu_interrupt,        /*  0x4B  0x0000012C   -   INT_CMP0                     */
        (isr)&cpu_interrupt,        /*  0x4C  0x00000130   -   INT_CMP1                     */
        (isr)&cpu_interrupt,        /*  0x4D  0x00000134   -   INT_CMP2                     */
        (isr)&cpu_interrupt,        /*  0x4E  0x00000138   -   INT_FTM0                     */
        (isr)&cpu_interrupt,        /*  0x4F  0x0000013C   -   INT_FTM1                     */
        (isr)&cpu_interrupt,        /*  0x50  0x00000140   -   INT_FTM2                     */
        (isr)&cpu_interrupt,        /*  0x51  0x00000144   -   INT_CMT                      */
        (isr)&cpu_interrupt,        /*  0x52  0x00000148   -   INT_RTC                      */
        (isr)&cpu_interrupt,        /*  0x53  0x0000014C   -   INT_RTC_Seconds              */
        (isr)&isr_clock64_tick,     /*  0x54  0x00000150   -   INT_PIT0                     */
        (isr)&cpu_interrupt,        /*  0x55  0x00000154   -   INT_PIT1                     */
        (isr)&cpu_interrupt,        /*  0x56  0x00000158   -   INT_PIT2                     */
        (isr)&cpu_interrupt,        /*  0x57  0x0000015C   -   INT_PIT3                     */
        (isr)&cpu_interrupt,        /*  0x58  0x00000160   -   INT_PDB0                     */
        (isr)&cpu_interrupt,        /*  0x59  0x00000164   -   INT_USB0                     */
        (isr)&cpu_interrupt,        /*  0x5A  0x00000168   -   INT_USBDCD                   */
        (isr)&cpu_interrupt,        /*  0x5B  0x0000016C   -   INT_Reserved91               */
        (isr)&cpu_interrupt,        /*  0x5C  0x00000170   -   INT_Reserved92               */
        (isr)&cpu_interrupt,        /*  0x5D  0x00000174   -   INT_Reserved93               */
        (isr)&cpu_interrupt,        /*  0x5E  0x00000178   -   INT_Reserved94               */
        (isr)&cpu_interrupt,        /*  0x5F  0x0000017C   -   INT_Reserved95               */
        (isr)&cpu_interrupt,        /*  0x60  0x00000180   -   INT_SDHC                     */
        (isr)&cpu_interrupt,        /*  0x61  0x00000184   -   INT_DAC0                     */
        (isr)&cpu_interrupt,        /*  0x62  0x00000188   -   INT_DAC1                     */
        (isr)&cpu_interrupt,        /*  0x63  0x0000018C   -   INT_TSI0                     */
        (isr)&cpu_interrupt,        /*  0x64  0x00000190   -   INT_MCG                      */
        (isr)&cpu_interrupt,        /*  0x65  0x00000194   -   INT_LPTimer                  */
        (isr)&cpu_interrupt,        /*  0x66  0x00000198   -   INT_LCD                      */
        (isr)&cpu_interrupt,        /*  0x67  0x0000019C   -   INT_PORTA                    */
        (isr)&cpu_interrupt,        /*  0x68  0x000001A0   -   INT_PORTB                    */
        (isr)&cpu_interrupt,        /*  0x69  0x000001A4   -   INT_PORTC                    */
        (isr)&cpu_interrupt,        /*  0x6A  0x000001A8   -   INT_PORTD                    */
        (isr)&cpu_interrupt,        /*  0x6B  0x000001AC   -   INT_PORTE                    */
        (isr)&cpu_interrupt,        /*  0x6C  0x000001B0   -   INT_Reserved108              */
        (isr)&cpu_interrupt,        /*  0x6D  0x000001B4   -   INT_Reserved109              */
        (isr)&cpu_interrupt,        /*  0x6E  0x000001B8   -   INT_SWI                      */
        (isr)&cpu_interrupt,        /*  0x6F  0x000001BC   -   INT_Reserved111              */
        (isr)&cpu_interrupt,        /*  0x70  0x000001C0   -   INT_Reserved112              */
        (isr)&cpu_interrupt,        /*  0x71  0x000001C4   -   INT_Reserved113              */
        (isr)&cpu_interrupt,        /*  0x72  0x000001C8   -   INT_Reserved114              */
        (isr)&cpu_interrupt,        /*  0x73  0x000001CC   -   INT_Reserved115              */
        (isr)&cpu_interrupt,        /*  0x74  0x000001D0   -   INT_Reserved116              */
        (isr)&cpu_interrupt,        /*  0x75  0x000001D4   -   INT_Reserved117              */
        (isr)&cpu_interrupt,        /*  0x76  0x000001D8   -   INT_Reserved118              */
        (isr)&cpu_interrupt         /*  0x77  0x000001DC   -   INT_Reserved119              */
    }
}; /* system_isr_table */
