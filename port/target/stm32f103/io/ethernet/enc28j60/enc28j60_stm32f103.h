/*
 * enc28j60_stm32f103.h
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
#ifndef _ENC28J60_STM32F103_H_
#define _ENC28J60_STM32F103_H_
#include <kernel.h>
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>

#ifndef CMAKE_BUILD
/* ENC28J60 device configuration. */
#define ENC28J60_STM32F103_RESET_DELAY  (100)
#define ENC28J60_STM32F407_BAUDRATE     (20000000)
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void enc28j60_stm32f103_init(void);
#if (ENC28J60_INT_POLL == FALSE)
void enc28j60_stm32f103_handle_interrupt(void);
void enc28j60_stm32f103_enable_interrupt(ENC28J60 *);
void enc28j60_stm32f103_disable_interrupt(ENC28J60 *);
#endif
uint8_t enc28j60_stm32f103_interrupt_pin(ENC28J60 *);
void enc28j60_stm32f103_reset(ENC28J60 *);
uint8_t *enc28j60_stm32f103_get_mac(ETH_DEVICE *);

#endif /* ETHERNET_ENC28J60 */
#endif /* _ENC28J60_STM32F103_H_ */
