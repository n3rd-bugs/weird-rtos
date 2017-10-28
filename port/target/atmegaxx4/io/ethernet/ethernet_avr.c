/*
 * ethernet_avr.c
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
#include <kernel.h>

#ifdef CONFIG_ETHERNET
#include <avr/interrupt.h>
#include <ethernet.h>
#include <ethernet_avr.h>
#ifdef ETHERNET_ENC28J60
#include <enc28j60_avr.h>
#endif
#include <string.h>

#if (ENC28J60_INT_POLL == FALSE)
/*
 * ISR(INT0_vect, ISR_NAKED)
 * This is interrupt callback for external interrupt signal 0.
 */
ISR(INT0_vect, ISR_NAKED)
{
    /* We have entered an ISR. */
    ISR_ENTER();

#ifdef ETHERNET_ENC28J60
    /* Handle enc28j60 interrupt. */
    enc28j60_avr_handle_interrupt();
#endif

    /* We are now exiting the ISR. */
    ISR_EXIT();

} /* ISR(INT0_vect, ISR_NAKED) */
#endif

/*
 * ethernet_avr_init
 * This function will initialize ethernet devices for AVR platform.
 */
void ethernet_avr_init(void)
{
#ifdef ETHERNET_ENC28J60
    /* Initialize ENC28j60 ethernet controller. */
    enc28j60_avr_init();
#endif

} /* ethernet_avr_init */

#endif /* CONFIG_ETHERNET */
