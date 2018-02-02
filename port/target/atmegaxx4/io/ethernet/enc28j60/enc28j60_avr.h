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

#ifndef CMAKE_BUILD
/* ENC28J60 device configuration. */
#define ENC28J60_AVR_RESET_DELAY            (100)
#define ENC28J60_USE_SPI_BB                 (FALSE)
#define ENC28J60_AVR_BAUDRATE               (21000000)

/* Bit-bang SPI interface configuration for ENC28J60 controller. */
#define ENC28J60_AVR_SPI_SS_BB              (4)
#define ENC28J60_AVR_SPI_PIN_SS_BB          (0x3)
#define ENC28J60_AVR_SPI_DDR_SS_BB          (0x4)
#define ENC28J60_AVR_SPI_PORT_SS_BB         (0x5)
#define ENC28J60_AVR_SPI_MOSI_BB            (5)
#define ENC28J60_AVR_SPI_PIN_MOSI_BB        (0x3)
#define ENC28J60_AVR_SPI_DDR_MOSI_BB        (0x4)
#define ENC28J60_AVR_SPI_PORT_MOSI_BB       (0x5)
#define ENC28J60_AVR_SPI_MISO_BB            (6)
#define ENC28J60_AVR_SPI_PIN_MISO_BB        (0x3)
#define ENC28J60_AVR_SPI_DDR_MISO_BB        (0x4)
#define ENC28J60_AVR_SPI_PORT_MISO_BB       (0x5)
#define ENC28J60_AVR_SPI_SCLK_BB            (7)
#define ENC28J60_AVR_SPI_PIN_SCLK_BB        (0x3)
#define ENC28J60_AVR_SPI_DDR_SCLK_BB        (0x4)
#define ENC28J60_AVR_SPI_PORT_SCLK_BB       (0x5)

/* Reset PIN configurations. */
#define ENC28J60_RST                        (4)
#define ENC28J60_RST_PORT                   (PORTD)
#define ENC28J60_RST_DDR                    (DDRD)
#define ENC28J60_RST_PIN                    (PIND)

/* Interrupt PIN configurations. */
#define ENC28J60_INT_SOURCE                 (INT0)
#define ENC28J60_INT                        (2)
#define ENC28J60_INT_PORT                   (PORTD)
#define ENC28J60_INT_DDR                    (DDRD)
#define ENC28J60_INT_PIN                    (PIND)
#endif /* CMAKE_BUILD */

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
