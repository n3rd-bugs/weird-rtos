/*
 * enc28j60_stm32f407.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#ifndef _ENC28J60_STM32F407_H_
#define _ENC28J60_STM32F407_H_
#include <os.h>
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>

/* ENC28J60 device configuration. */
#define ENC28J60_STM32F407_RESET_DELAY  (100)

#define STM32_UUID                      ((uint8_t *)0x1FFF7A10)

/* Function prototypes. */
void enc28j60_stm32f407_init();
#if (ENC28J60_INT_POLL == FALSE)
void enc28j60_stm32f407_handle_interrupt();
void enc28j60_stm32f407_enable_interrupt(ENC28J60 *);
void enc28j60_stm32f407_disable_interrupt(ENC28J60 *);
#endif
uint8_t enc28j60_stm32f407_interrupt_pin(ENC28J60 *);
void enc28j60_stm32f407_reset(ENC28J60 *);
uint8_t *enc28j60_stm32f407_get_mac(ETH_DEVICE *);

#endif /* ETHERNET_ENC28J60 */
#endif /* _ENC28J60_STM32F407_H_ */
