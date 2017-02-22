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
#include <console.h>
#ifdef NET_DHCP
#include <net_dhcp.h>
#endif
#ifdef DHCP_CLIENT
#include <net_dhcp_client.h>
#endif

/* Error code definitions. */
#define ENC28J60_DISCONNECTED       -11000

/* ENC28J60 watch dog configuration. */
#define ENC28J60_WDT_TIMEOUT        (OS_TICKS_PER_SEC / 10)

/* ENC28J60 CLKRDY delay configuration. */
#define ENC28J60_CLKRDY_TIMEOUT     (100)
#define ENC28J60_CLKRDY_DELAY       (20)

/* ENC28J60 configurations. */
#define ENC28J60_CONTINUE_READ      TRUE
#define ENC28J60_INT_POLL           TRUE

/* ENC28J60 device configuration. */
#define ENC28J60_REV_ID             (0x06)

/* ENC28J60 device flag definitions. */
#define ENC28J60_IN_TX              (0x01)
#define ENC28J60_INT_ENABLE         (0x02)

/* Buffer configuration for a enc28j60 device. */
#define ENC28J60_MAX_BUFFER_SIZE    (256)
#define ENC28J60_NUM_BUFFERS        (32)
#define ENC28J60_NUM_BUFFER_LISTS   (24)
#define ENC28J60_NUM_THR_BUFFER     (16)

/* Networking configuration for a enc28j60 device. */
#define ENC28J60_NUM_ARP            (4)
#define ENC28J60_NUM_IPV4_FRAGS     (2)

/* ENC28J60 receive packet definitions. */
#define ENC28J60_RX_HEAD_SIZE       (6)
#define ENC28J60_RX_CRC_LEN         (4)
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

/* ENC28J60 transmit packet definitions. */
#define ENC28J60_TX_HEAD_SIZE       (1 + 6)
#define ENC28J60_TX_MIN_BUF_SIZE    (ETH_MTU_SIZE + ENC28J60_TX_HEAD_SIZE)

/* ENC28J60 RX/TX FIFO configuration. */
#define ENC28J60_FIFO_SIZE          (0x2000)
#define ENC28J60_RX_START           (0)
#define ENC28J60_RX_END             (((ENC28J60_FIFO_SIZE - ENC28J60_TX_MIN_BUF_SIZE) & 0xFFFE) - 1)
#define ENC28J60_TX_START           ((ENC28J60_FIFO_SIZE - ENC28J60_TX_MIN_BUF_SIZE) & 0xFFFE)
#define ENC28J60_TX_END             (ENC28J60_FIFO_SIZE - 1)

/* RX pointer calculation macro. */
#define ENC28J60_RX_PTR(p)          ((((p - 1) < ENC28J60_RX_START) || ((p - 1) > ENC28J60_RX_END)) ? ENC28J60_RX_END : (p - 1))
#define ENC28J60_RX_START_PTR(p)    (((p + ENC28J60_RX_HEAD_SIZE) > ENC28J60_RX_END) ? ((p + ENC28J60_RX_HEAD_SIZE) - (ENC28J60_RX_END - (ENC28J60_RX_START - 1))) : (p + ENC28J60_RX_HEAD_SIZE))

/* MAC address definitions. */
#define ENC28J60_OUI_B0             0x00
#define ENC28J60_OUI_B1             0x04
#define ENC28J60_OUI_B2             0xA3

/* ENC28J60 device structure definition. */
typedef struct _enc28j60_device ENC28J60;

/* BSP API's. */
typedef void (ENC28J60_ENABLE_INT) (ENC28J60 *);
typedef void (ENC28J60_DISABLE_INT) (ENC28J60 *);
typedef uint8_t (ENC28J60_INTERRUPT_PIN) (ENC28J60 *);
typedef void (ENC28J60_RESET) (ENC28J60 *);
typedef uint8_t *(ENC28J60_GET_MAC) (ETH_DEVICE *);

/* ENC28J60 device structure. */
struct _enc28j60_device
{
    /* Ethernet device structure. */
    ETH_DEVICE  ethernet_device;

#ifdef DHCP_CLIENT
    /* DHCP client data. */
    DHCP_CLIENT_DEVICE  dhcp_client;
#endif

#ifdef IPV4_ENABLE_FRAG
    /* IPv4 fragment list. */
    IPV4_FRAGMENT   ipv4_fragments[ENC28J60_NUM_IPV4_FRAGS];
#endif

#ifdef NET_ARP
    /* ARP entry list. */
    ARP_ENTRY   arp_entries[ENC28J60_NUM_ARP];
#endif

    /* Device APIs. */
    ENC28J60_ENABLE_INT     *enable_interrupts;
    ENC28J60_DISABLE_INT    *disable_interrupts;
    ENC28J60_INTERRUPT_PIN  *interrupt_pin;
    ENC28J60_RESET          *reset;
    ENC28J60_GET_MAC        *get_mac;

    /* Buffer used to manage data for this device. */
    uint8_t     buffer[ENC28J60_MAX_BUFFER_SIZE * ENC28J60_NUM_BUFFERS];

    /* File system buffers. */
    FS_BUFFER_DATA  fs_buffer_data;
    FS_BUFFER_ONE   fs_buffer[ENC28J60_NUM_BUFFERS];
    FS_BUFFER       fs_buffer_list[ENC28J60_NUM_BUFFER_LISTS];

    /* SPI device structure. */
    SPI_DEVICE  spi;

    /* Current receive pointer. */
    uint16_t    rx_ptr;

    /* Current selected memory block. */
    uint8_t     mem_block;

    /* Device flags. */
    uint8_t     flags;

    /* Structure padding. */
    uint8_t     pad[4];

};

/* Function prototypes. */
void enc28j60_init(ENC28J60 *);

#endif /* ETHERNET_ENC28J60 */
#endif /* _ENC28J60_H_ */
