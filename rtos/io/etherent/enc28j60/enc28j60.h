/*
 * enc28j60.h
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
#ifndef _ENC28J60_H_
#define _ENC28J60_H_
#include <kernel.h>
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
#include <idle.h>

/* Error code definitions. */
#define ENC28J60_DISCONNECTED       -11000

#ifdef CMAKE_BUILD
#include <enc28j60_config.h>
#else
/* ENC28J60 watch dog configuration. */
#define ENC28J60_WDT_TIMEOUT        (1000)

/* ENC28J60 CLKRDY delay configuration. */
#define ENC28J60_CLKRDY_TIMEOUT     (100)
#define ENC28J60_CLKRDY_DELAY       (20)

/* ENC28J60 configurations. */
#define ENC28J60_CONTINUE_READ      TRUE
#define ENC28J60_INT_POLL           FALSE

/* ENC28J60 device configuration. */
#define ENC28J60_REV_ID             (0x06)

/* Buffer configuration for a enc28j60 device. */
#define ENC28J60_MAX_BUFFER_SIZE    (32)
#define ENC28J60_NUM_BUFFERS        (32)
#define ENC28J60_NUM_BUFFER_LISTS   (16)
#define ENC28J60_NUM_THR_BUFFER     (8)
#define ENC28J60_NUM_THR_LIST       (4)

/* Networking configuration for a enc28j60 device. */
#define ENC28J60_NUM_ARP            (4)
#define ENC28J60_NUM_IPV4_FRAGS     (2)

/* Default IP configuration. */
#define ENC28J60_DEFAULT_IP         (0xC0A80032)
#define ENC28J60_DEFAULT_SUBNET     (0xFFFFFF00)
#endif /* CMAKE_BUILD */

/* ENC28J60 device flag definitions. */
#define ENC28J60_IN_TX              (0x01)
#define ENC28J60_INT_ENABLE         (0x02)

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

/* ENC28J60 transmit packet definitions. */
#define ENC28J60_TX_HEAD_SIZE       (1 + 6)
#define ENC28J60_TX_MIN_BUF_SIZE    (ETH_MTU_SIZE + ENC28J60_TX_HEAD_SIZE)

/* ENC28J60 RX/TX FIFO configuration. */
#define ENC28J60_CRC_LEN            (4)
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

#if (ENC28J60_INT_POLL == TRUE)
#if (IDLE_WORK_MAX == 0)
#error "At least one idle work is required to enable polling mode."
#endif /* (IDLE_WORK_MAX == 0) */
#endif /* ENC28J60_INT_POLL */

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

    /* File system buffers. */
    FS_BUFFER_DATA  fs_buffer_data;
    uint8_t         buffer[ENC28J60_MAX_BUFFER_SIZE * ENC28J60_NUM_BUFFERS];
    FS_BUFFER       fs_buffer[ENC28J60_NUM_BUFFERS];
    FS_BUFFER_LIST  fs_list_free[ENC28J60_NUM_BUFFER_LISTS];

    /* SPI device structure. */
    SPI_DEVICE  spi;

    /* Current receive pointer. */
    uint16_t    rx_ptr;

    /* Current selected memory block. */
    uint8_t     mem_block;

    /* Device flags. */
    uint8_t     flags;

};

/* Function prototypes. */
void enc28j60_init(ENC28J60 *);

#endif /* ETHERNET_ENC28J60 */
#endif /* _ENC28J60_H_ */
