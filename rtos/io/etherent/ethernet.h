/*
 * ethernet.h
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
#ifndef _ETHERNET_H_
#define _ETHERNET_H_
#include <os.h>

#ifdef CONFIG_ETHERNET
#include <fs.h>
#include <fs_buffer.h>
#include <net.h>
#include <net_device.h>

/* Ethernet configuration. */
#define ETHERNET_ENC28J60

/* Ethernet error definitions. */
#define ETH_TX_BLOCKED      -1100

/* Ethernet header definitions. */
#define ETH_HDR_DST_OFFSET  (0)
#define ETH_HDR_SRC_OFFSET  (6)
#define ETH_HDR_TYPE_OFFSET (12)

/* Ethernet definitions. */
#define ETH_ADDR_LEN        (6)
#define ETH_PROTO_LEN       (2)
#define ETH_MTU_SIZE        (1522)

/* Ethernet MAC address definitions. */
#define ETH_MAC_OUI         (0x02)
#define ETH_MAC_MULTICAST   (0x01)

/* Ethernet protocol definitions. */
#define ETH_PROTO_IP        (0x0800)
#define ETH_PROTO_ARP       (0x0806)

/* Ethernet device flags. */
#define ETH_FLAG_INIT       0x01
#define ETH_FLAG_INT        0x02
#define ETH_FLAG_TX         0x04

/* Ethernet address definitions. */
#define ETH_UNSPEC_ADDR     ((uint8_t []){0, 0, 0, 0, 0, 0})
#define ETH_BCAST_ADDR      ((uint8_t []){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF})

/* Ethernet device API. */
typedef void ETH_INIT (void *);
typedef void ETH_INTERRUPT (void *);
typedef int32_t ETH_TRANSMIT (void *, FS_BUFFER *);

/* Include ethernet target configurations. */
#include <ethernet_target.h>
#ifdef NET_ARP
#include <net_arp.h>
#endif

/* Ethernet device data structure. */
typedef struct _eth_device
{
    /* File descriptor for this ethernet device. */
    FS          fs;

    /* Networking device associated with ethernet device. */
    NET_DEV     net_device;

#ifdef CONFIG_SEMAPHORE
    /* Lock for this device instance. */
    SEMAPHORE   lock;
#endif

    /* TX queue suspend for this device. */
    FS_PARAM    fs_param;
    SUSPEND     suspend;

#ifdef NET_ARP
    /* ARP device data. */
    ARP_DATA    arp;
#endif

    /* Ethernet driver hooks. */
    ETH_INIT        *initialize;
    ETH_INTERRUPT   *interrupt;
    ETH_TRANSMIT    *transmit;

    /* MAC address assigned to this device. */
    uint8_t     mac[ALLIGN_CEIL(ETH_ADDR_LEN)];

    /* Device flags. */
    uint8_t     flags;

    /* Structure padding. */
    uint8_t     pad[3];

} ETH_DEVICE;

/* Function prototypes. */
void ethernet_init();
void ethernet_regsiter(ETH_DEVICE *, ETH_INIT *, ETH_TRANSMIT *, ETH_INTERRUPT *);
uint8_t *ethernet_random_mac(ETH_DEVICE *);
uint8_t *ethernet_get_mac_address(FD);
void ethernet_interrupt(ETH_DEVICE *);
int32_t ethernet_buffer_receive(FS_BUFFER *);

#endif /* CONFIG_ETHERNET */

#endif /* _ETHERNET_H_ */
