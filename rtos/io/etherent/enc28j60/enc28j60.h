/*
 * enc28j60.h
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
#ifndef _ENC28J60_H_
#define _ENC28J60_H_
#include <os.h>
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#ifndef CONFIG_NET
#error "Networking stack required by ENC28j60."
#endif
#ifndef CONFIG_SPI
#error "SPI required by ENC28j60."
#endif
#include <spi.h>
#include <condition.h>
#include <net_device.h>

/* Error code definitions. */
#define ENC28J60_SPI_ERROR      -1100

/* ENC28J60 device flags. */
#define ENC28J60_FLAG_INIT      0x01
#define ENC28J60_FLAG_INT       0x02

/* ENC28J60 device configuration. */
#define ENC28J60_REV_ID         (0x06)
#define ENC28J60_MTU            (1518)

/* ENC28J60 receive packet definitions. */
#define ENC28J60_RX_HEAD_SIZE       (6)
#define ENC28J60_RX_RXLONGEVDROPEV  (0x0001)
#define ENC28J60_RX_CARRIEREV       (0x0004)
#define ENC28J60_RX_CRCERROR        (0x0010)
#define ENC28J60_RX_LENCHECKERR     (0x0020)
#define ENC28J60_RX_LENOUTOFRANGE   (0x0040)
#define ENC28J60_RX_RXOK            (0x0080)
#define ENC28J60_RX_RXMULTICAST     (0x0100)
#define ENC28J60_RX_RXBROADCAST     (0x0200)
#define ENC28J60_RX_DRIBBLENIBBLE   (0x0400)
#define ENC28J60_RX_RXCONTROLFRAME  (0x0800)
#define ENC28J60_RX_RXPAUSEFRAME    (0x1000)
#define ENC28J60_RX_RXUNKNOWNOPCODE (0x2000)
#define ENC28J60_RX_RXTYPEVLAN      (0x4000)

/* ENC28J60 RX/TX FIFO configuration. */
#define ENC28J60_FIFO_SIZE      (0x1FFF)
#define ENC28J60_RX_START       (0)
#define ENC28J60_RX_END         ((((ENC28J60_FIFO_SIZE - ENC28J60_MTU) + 1) & 0xFFFE) - 1)
#define ENC28J60_TX_START       (((ENC28J60_FIFO_SIZE - ENC28J60_MTU) + 1) & 0xFFFE)
#define ENC28J60_TX_END         (ENC28J60_FIFO_SIZE)

/* RX pointer calculation macro. */
#define ENC28J60_RX_PTR(p)          ((((p - 1) < ENC28J60_RX_START) || ((p - 1) > ENC28J60_RX_END)) ? ENC28J60_RX_END : (p - 1))
#define ENC28J60_RX_START_PTR(p)    (((p + ENC28J60_RX_HEAD_SIZE) > ENC28J60_RX_END) ? ((p + ENC28J60_RX_HEAD_SIZE) - (ENC28J60_RX_END - (ENC28J60_RX_START - 1))) : (p + ENC28J60_RX_HEAD_SIZE))

/* ENC28J60 device structure. */
typedef struct _enc28j60_device
{
    /* SPI device structure. */
    SPI_DEVICE  spi;

    /* Networking condition for this device. */
    CONDITION   condition;

    /* Suspend structure to be used. */
    SUSPEND     suspend;

    /* Networking device associated with enc28j60 device. */
    NET_DEV     net_device;

    /* Current receive pointer. */
    uint16_t    rx_ptr;

    /* Current selected memory block. */
    uint8_t     mem_block;

    /* Device flags for enc28j60 device. */
    uint8_t     flags;

} ENC28J60;

/* Include target configuration. */
#include <enc28j60_target.h>

/* Function prototypes. */
void enc28j60_init(ENC28J60 *);

#endif /* ETHERNET_ENC28J60 */
#endif /* _ENC28J60_H_ */
