/*
 * enc28j60_avr.h
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
#ifndef _ENC28J60_AVR_H_
#define _ENC28J60_AVR_H_
#include <kernel.h>
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>
#include <enc28j60_avr_config.h>

/* Function prototypes. */
void enc28j60_avr_init(void);
#if (ENC28J60_INT_POLL == FALSE)
void enc28j60_avr_handle_interrupt(void);
void enc28j60_avr_enable_interrupt(ENC28J60 *);
void enc28j60_avr_disable_interrupt(ENC28J60 *);
#endif
uint8_t enc28j60_avr_interrupt_pin(ENC28J60 *);
void enc28j60_avr_reset(ENC28J60 *);
uint8_t *enc28j60_avr_get_mac(ETH_DEVICE *);

#endif /* ETHERNET_ENC28J60 */
#endif /* _ENC28J60_AVR_H_ */
