/*
 * ethernet_avr.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
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
