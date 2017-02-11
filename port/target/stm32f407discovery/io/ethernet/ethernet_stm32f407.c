/*
 * ethernet_stm32f407.c
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

#ifdef CONFIG_ETHERNET
#include <ethernet.h>
#include <ethernet_stm32f407.h>
#ifdef ETHERNET_ENC28J60
#include <enc28j60_stm32f407.h>
#endif
#include <string.h>

#if (ENC28J60_INT_POLL == FALSE)
/*
 * exti2_interrupt
 * This function is interrupt handler for EXTI2 interrupt.
 */
ISR_FUN exti2_interrupt()
{
    OS_ISR_ENTER();

#ifdef ETHERNET_ENC28J60
    /* Handle enc28j60 interrupt. */
    enc28j60_stm32f407_handle_interrupt();
#endif

    /* Clear the interrupt pending bit or EXTI2 channel. */
    EXTI->PR = 0x04;

    OS_ISR_EXIT();

} /* exti2_interrupt */
#endif

/*
 * ethernet_stm32f407_init
 * This function will initialize ethernet devices for stm32f407 platform.
 */
void ethernet_stm32f407_init()
{
#ifdef ETHERNET_ENC28J60
    /* Initialize ENC28j60 ethernet controller. */
    enc28j60_stm32f407_init();
#endif

} /* ethernet_stm32f407_init */

#endif /* CONFIG_ETHERNET */
