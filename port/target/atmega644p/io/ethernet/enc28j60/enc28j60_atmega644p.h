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
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _ENC28J60_ATMEGA644P_H_
#define _ENC28J60_ATMEGA644P_H_
#include <kernel.h>
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>

#ifndef CMAKE_BUILD
/* ENC28J60 device configuration. */
#define ENC28J60_ATMEGA644P_RESET_DELAY         (100)
#define ENC28J60_USE_SPI_BB                     (FALSE)

/* Bit-bang SPI interface configuration for ENC28J60 controller. */
#define ENC28J60_ATMEGA644P_SPI_SS_BB           (4)
#define ENC28J60_ATMEGA644P_SPI_PIN_SS_BB       (0x3)
#define ENC28J60_ATMEGA644P_SPI_DDR_SS_BB       (0x4)
#define ENC28J60_ATMEGA644P_SPI_PORT_SS_BB      (0x5)
#define ENC28J60_ATMEGA644P_SPI_MOSI_BB         (5)
#define ENC28J60_ATMEGA644P_SPI_PIN_MOSI_BB     (0x3)
#define ENC28J60_ATMEGA644P_SPI_DDR_MOSI_BB     (0x4)
#define ENC28J60_ATMEGA644P_SPI_PORT_MOSI_BB    (0x5)
#define ENC28J60_ATMEGA644P_SPI_MISO_BB         (6)
#define ENC28J60_ATMEGA644P_SPI_PIN_MISO_BB     (0x3)
#define ENC28J60_ATMEGA644P_SPI_DDR_MISO_BB     (0x4)
#define ENC28J60_ATMEGA644P_SPI_PORT_MISO_BB    (0x5)
#define ENC28J60_ATMEGA644P_SPI_SCLK_BB         (7)
#define ENC28J60_ATMEGA644P_SPI_PIN_SCLK_BB     (0x3)
#define ENC28J60_ATMEGA644P_SPI_DDR_SCLK_BB     (0x4)
#define ENC28J60_ATMEGA644P_SPI_PORT_SCLK_BB    (0x5)
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void enc28j60_atmega644p_init(void);
#if (ENC28J60_INT_POLL == FALSE)
void enc28j60_atmega644p_handle_interrupt(void);
void enc28j60_atmega644p_enable_interrupt(ENC28J60 *);
void enc28j60_atmega644p_disable_interrupt(ENC28J60 *);
#endif
uint8_t enc28j60_atmega644p_interrupt_pin(ENC28J60 *);
void enc28j60_atmega644p_reset(ENC28J60 *);
uint8_t *enc28j60_atmega644p_get_mac(ETH_DEVICE *);

#endif /* ETHERNET_ENC28J60 */
#endif /* _ENC28J60_ATMEGA644P_H_ */
