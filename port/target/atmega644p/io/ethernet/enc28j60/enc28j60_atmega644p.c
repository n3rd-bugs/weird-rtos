/*
 * enc28j60_atmega644p.c
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
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>
#include <enc28j60_atmega644p.h>
#include <string.h>

/* ENC28J60 device instance. */
static ENC28J60 enc28j60;

/*
 * enc28j60_atmega644p_init
 * This function will initialize enc28j60 device for atmega644p platform.
 */
void enc28j60_atmega644p_init()
{
    /* Clear the enc28j60 device structure. */
    memset(&enc28j60, 0, sizeof(ENC28J60));

    /* Set RST (PD.4) as output and INT (PD.2) as input. */
    DDRD |= (1 << 4);
    DDRD &= (uint8_t)~(1 << 2);

    /* Disable INT0 interrupt. */
    EIMSK &= (uint8_t)~(1 << 0);

    /* Setup INT0 to raise an interrupt on falling edge. */
    EICRA &= (uint8_t)~(0x03 << 0);
    EICRA |= (0x02 << 0);

    /* Initialize name for this device. */
    enc28j60.ethernet_device.fs.name = "\\ethernet\\enc28j60";

    /* Do enc28j60 initialization. */
    enc28j60_init(&enc28j60);

} /* enc28j60_atmega644p_init */

/*
 * enc28j60_atmega644p_handle_interrupt
 * This function will handle interrupt for given enc28j60 device.
 */
void enc28j60_atmega644p_handle_interrupt()
{
    /* Disable interrupt until we process it. */
    enc28j60_atmega644p_disable_interrupt(&enc28j60);

    /* Handle interrupt for this device. */
    ethernet_interrupt(&enc28j60.ethernet_device);

} /* enc28j60_atmega644p_handle_interrupt */

/*
 * enc28j60_atmega644p_enable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  enabled.
 * This function will enable interrupt for given enc28j60 device.
 */
void enc28j60_atmega644p_enable_interrupt(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Enable INT0 to process enc28j60 device interrupts. */
    EIMSK |= (1 << 0);

} /* enc28j60_atmega644p_enable_interrupt */

/*
 * enc28j60_atmega644p_disable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  disabled.
 * This function will disable interrupt for given enc28j60 device.
 */
void enc28j60_atmega644p_disable_interrupt(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Disable INT0 interrupt. */
    EIMSK &= (uint8_t)~(1 << 0);

} /* enc28j60_atmega644p_disable_interrupt */

/*
 * enc28j60_atmega644p_reset
 * device: ENC28J60 device needed to be reset.
 * This function will reset the target enc28j60 device.
 */
void enc28j60_atmega644p_reset(ENC28J60 *device)
{
    FD *fd = (FD)device;

    /* Clear the RST, i.e. PD.4. */
    PORTD &= (uint8_t)~(1 << 4);

    /* Release lock for this device. */
    fd_release_lock(fd);

    /* Sleep to wait for target to actually reset. */
    sleep_ms(ENC28J60_ATMEGA644P_RESET_DELAY);

    /* Acquire lock for this device. */
    OS_ASSERT(fd_get_lock(fd) != SUCCESS);

    /* Set the RST, i.e. PD.4. */
    PORTD |= (1 << 4);

} /* enc28j60_atmega644p_reset */

#endif /* ETHERNET_ENC28J60 */