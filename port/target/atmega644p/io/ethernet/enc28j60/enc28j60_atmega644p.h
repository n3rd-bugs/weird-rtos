/*
 * enc28j60_atmega644p.h
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
#ifndef _ENC28J60_ATMEGA644P_H_
#define _ENC28J60_ATMEGA644P_H_
#include <os.h>
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>

/* ENC28J60 device configuration. */
#define ENC28J60_ATMEGA644P_RESET_DELAY     (100)

/* Function prototypes. */
void enc28j60_atmega644p_init();
void enc28j60_atmega644p_handle_interrupt();
void enc28j60_atmega644p_enable_interrupt(ENC28J60 *);
void enc28j60_atmega644p_disable_interrupt(ENC28J60 *);
void enc28j60_atmega644p_reset(ENC28J60 *);
uint8_t *enc28j60_atmega644p_get_mac(ETH_DEVICE *);

#endif /* ETHERNET_ENC28J60 */
#endif /* _ENC28J60_ATMEGA644P_H_ */
