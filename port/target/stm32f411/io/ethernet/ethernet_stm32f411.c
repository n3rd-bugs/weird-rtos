/*
 * ethernet_stm32f411.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef CONFIG_ETHERNET
#include <ethernet.h>
#include <ethernet_stm32f411.h>
#ifdef ETHERNET_ENC28J60
#include <enc28j60_stm32f411.h>
#endif
#include <string.h>

#if (ENC28J60_INT_POLL == FALSE)
/*
 * exti2_interrupt
 * This function is interrupt handler for EXTI2 interrupt.
 */
ISR_FUN exti2_interrupt(void)
{
    ISR_ENTER();

#ifdef ETHERNET_ENC28J60
    /* Handle enc28j60 interrupt. */
    enc28j60_stm32f411_handle_interrupt();
#endif

    /* Clear the interrupt pending bit or EXTI2 channel. */
    EXTI->PR = 0x4;

    ISR_EXIT();

} /* exti2_interrupt */
#endif

/*
 * ethernet_stm32f411_init
 * This function will initialize ethernet devices for stm32f411 platform.
 */
void ethernet_stm32f411_init(void)
{
#ifdef ETHERNET_ENC28J60
    /* Initialize ENC28j60 ethernet controller. */
    enc28j60_stm32f411_init();
#endif

} /* ethernet_stm32f411_init */

#endif /* CONFIG_ETHERNET */
