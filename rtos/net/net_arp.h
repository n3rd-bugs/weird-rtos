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
#include <condition.h>

/* ARP entry flags. */
#define ARP_FLAG_VALID          0x01
#define ARP_FLAG_UP             0x02
#define ARP_FLAG_IN_USE         0x04

/* ARP configuration. */
#define ARP_TIMEOUT             (1 * OS_TICKS_PER_SEC)
#define ARP_RETRY_COUNT         (3)
#define ARP_UPDATE_TIME         (5 * OS_TICKS_PER_SEC)

/* ARP header definitions. */
#define ARP_HDR_LEN             (28)
#define ARP_HDR_PRE_LEN         (8)
#define ARP_HDR_SRC_HW_OFFSET   (0)
#define ARP_HDR_SRC_IPV4_OFFSET (6)
#define ARP_HDR_TGT_HW_OFFSET   (10)
#define ARP_HDR_TGT_IPV4_OFFSET (16)

/* ARP protocol definitions. */
#define ARP_ETHER_TYPE          (1)
#define ARP_PROTO_IP            (ETH_PROTO_IP)

/* ARP operation definitions. */
#define ARP_OP_REQUEST          (1)
#define ARP_OP_RESPONSE         (2)

/* ARP entry data. */
typedef struct _arp_entry
{
    /* Buffer list for the buffers waiting on this ARP entry. */
    struct _arp_buffer_list
    {
        FS_BUFFER       *head;
        FS_BUFFER       *tail;
    } buffer_list;

    /* Tick at which we will be routing this entry again. */
    uint64_t    next_timeout;

    /* IP address for this ARP entry. */
    uint32_t    ip;

    /* Ethernet MAC address for the destination IP address. */
    uint8_t     mac[ALLIGN_CEIL(ETH_ADDR_LEN)];

    /* Number of retries we have sent for this ARP entry. */
    uint8_t     retry_count;

    /* ARP entry flags. */
    uint8_t     flags;

    /* Structure padding. */
    uint8_t     pad[2];

} ARP_ENTRY;

/* ARP device data. */
typedef struct _arp_data
{
    /* Condition data to process ARP events for this device. */
    CONDITION   condition;
    SUSPEND     suspend;

    /* Array of ARP entries for a device. */
    ARP_ENTRY   *entries;

    /* Number of ARP entries for this device. */
    uint32_t    num_entries;

} ARP_DATA;

/* Function prototypes. */
void arp_set_data(FD, ARP_ENTRY *, uint32_t);
ARP_DATA *arp_get_data(FD);
int32_t net_process_arp(FS_BUFFER *);

#endif /* NET_ARP */
#endif /* CONFIG_NET */
#endif /* _NET_ARP_H_ */
