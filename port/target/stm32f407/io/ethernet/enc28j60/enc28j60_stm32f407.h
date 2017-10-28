/*
 * enc28j60_stm32f407.h
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
#ifndef _ENC28J60_STM32F407_H_
#define _ENC28J60_STM32F407_H_
#include <kernel.h>
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>

#ifndef CMAKE_BUILD
/* ENC28J60 device configuration. */
#define ENC28J60_STM32F407_RESET_DELAY  (100)
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void enc28j60_stm32f407_init(void);
#if (ENC28J60_INT_POLL == FALSE)
void enc28j60_stm32f407_handle_interrupt(void);
void enc28j60_stm32f407_enable_interrupt(ENC28J60 *);
void enc28j60_stm32f407_disable_interrupt(ENC28J60 *);
#endif
uint8_t enc28j60_stm32f407_interrupt_pin(ENC28J60 *);
void enc28j60_stm32f407_reset(ENC28J60 *);
uint8_t *enc28j60_stm32f407_get_mac(ETH_DEVICE *);

#endif /* ETHERNET_ENC28J60 */
#endif /* _ENC28J60_STM32F407_H_ */
