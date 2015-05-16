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
#include <net_device.h>

/* Ethernet configuration. */
#define ETHERNET_ENC28J60

/* Ethernet definitions. */
#define ETH_ADDR_LEN        (6)
#define ETH_PROTO_LEN       (2)
#define ETH_MTU_SIZE        (1522)

/* Ethernet protocol definitions. */
#define ETH_PROTO_IP        (0x0800)

/* Ethernet device flags. */
#define ETH_FLAG_INIT      0x01
#define ETH_FLAG_INT       0x02

/* Ethernet device API. */
typedef void ETH_INIT (void *);
typedef void ETH_INTERRUPT (void *);

/* Include ethernet target configurations. */
#include <ethernet_target.h>

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

    /* Ethernet driver hooks. */
    ETH_INIT        *initialize;
    ETH_INTERRUPT   *interrupt;

    /* Device flags for enc28j60 device. */
    uint8_t     flags;

    /* Structure padding. */
    uint8_t     pad[3];

} ETH_DEVICE;

/* Function prototypes. */
void ethernet_init();
void ethernet_regsiter(ETH_DEVICE *, ETH_INIT *, ETH_INTERRUPT *);
void ethernet_interrupt(ETH_DEVICE *);
int32_t ethernet_buffer_receive(FS_BUFFER *);

#endif /* ETHERNET_ENC28J60 */

#endif /* _ETHERNET_H_ */
