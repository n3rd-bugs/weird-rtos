/*
 * ethernet_atmega644p.c
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
#include <avr/interrupt.h>
#include <ethernet.h>
#include <ethernet_atmega644p.h>
#ifdef ETHERNET_ENC28J60
#include <enc28j60_atmega644p.h>
#endif
#include <string.h>

/*
 * ISR(TIMER1_COMPA_vect, ISR_NAKED)
 * This is interrupt callback for external interrupt signal 0.
 */
ISR(SIG_INTERRUPT0, ISR_NOBLOCK)
{
    OS_ISR_ENTER();

#ifdef ETHERNET_ENC28J60
    /* Handle enc28j60 interrupt. */
    enc28j60_atmega644p_handle_interrupt();
#endif

    OS_ISR_EXIT();

} /* ISR(SIG_INTERRUPT0, ISR_NOBLOCK) */

/*
 * ethernet_atmega644p_init
 * This function will initialize ethernet devices for atmega644p platform.
 */
void ethernet_atmega644p_init()
{
#ifdef ETHERNET_ENC28J60
    /* Initialize ENC28j60 ethernet controller. */
    enc28j60_atmega644p_init();
#endif

} /* ethernet_atmega644p_init */

#endif /* CONFIG_ETHERNET */