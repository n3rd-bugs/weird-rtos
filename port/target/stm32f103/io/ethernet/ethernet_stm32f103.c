/*
 * ethernet_stm32f103.c
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
#include <kernel.h>

#ifdef IO_ETHERNET
#include <ethernet.h>
#include <ethernet_stm32f103.h>
#ifdef ETHERNET_ENC28J60
#include <enc28j60_stm32f103.h>
#endif
#include <string.h>

#if (ENC28J60_INT_POLL == FALSE)
/*
 * exti15_10_interrupt
 * This function is interrupt handler for EXTI15_10 interrupt.
 */
ISR_FUN exti15_10_interrupt(void)
{
    ISR_ENTER();

#ifdef ETHERNET_ENC28J60
    /* Handle enc28j60 interrupt. */
    enc28j60_stm32f103_handle_interrupt();
#endif

    /* Clear the interrupt pending bit or EXTI14 channel. */
    EXTI->PR = (1 << 14);

    ISR_EXIT();

} /* exti15_10_interrupt */
#endif

/*
 * ethernet_stm32f103_init
 * This function will initialize ethernet devices for stm32f103 platform.
 */
void ethernet_stm32f103_init(void)
{
#ifdef ETHERNET_ENC28J60
    /* Initialize ENC28j60 ethernet controller. */
    enc28j60_stm32f103_init();
#endif

} /* ethernet_stm32f103_init */

#endif /* IO_ETHERNET */
