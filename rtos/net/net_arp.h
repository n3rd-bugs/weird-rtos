/*
 * net_arp.h
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
#ifndef _NET_ARP_H_
#define _NET_ARP_H_
#include <os.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_ARP
#ifndef CONFIG_ETHERNET
#error "Ethernet driver is required for ARP"
#endif
#ifndef NET_IPV4
#error "IPv4 stack is required for ARP."
#endif
#include <ethernet.h>

/* ARP entry flags. */
#define ARP_FLAG_VALID      0x01

/* ARP header definitions. */
#define ARP_ETHER_TYPE      (1)
#define ARP_PROTO_IP        (ETH_PROTO_IP)

/* ARP entry data. */
typedef struct _arp_entry
{
    /* IP address for this ARP entry. */
    uint32_t    ip;

    /* Ethernet MAC address for the destination IP address. */
    uint8_t     mac[ALLIGN_CEIL(ETH_ADDR_LEN)];

    /* ARP entry flags. */
    uint8_t     flags;

    /* Structure padding. */
    uint8_t     pad[3];

} ARP_ENTRY;

/* ARP device data. */
typedef struct _arp_data
{
    /* Array of ARP entries for a device. */
    ARP_ENTRY   *entries;

    /* Number of ARP entries for this device. */
    uint32_t    num_entries;

} ARP_DATA;

/* Function prototypes. */
int32_t net_process_arp(FS_BUFFER *);

#endif /* NET_ARP */
#endif /* CONFIG_NET */
#endif /* _NET_ARP_H_ */
