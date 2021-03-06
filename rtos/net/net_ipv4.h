/*
 * net_ipv4.h
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
#ifndef _NET_IPv4_H_
#define _NET_IPv4_H_
#include <kernel.h>

#ifdef CONFIG_NET
#ifdef NET_IPV4

#ifdef NET_DHCP
#include <net_dhcp.h>
#endif

#include <net_ipv4_config.h>

#ifdef DHCP_CLIENT
typedef struct _dhcp_client_device DHCP_CLIENT_DEVICE;
#endif


/* Protocol definitions. */
#define IP_PROTO_ICMP               (0x1)
#define IP_PROTO_TCP                (0x6)
#define IP_PROTO_UDP                (0x11)

/* IPv4 header definitions. */
#define IPV4_ADDR_LEN               (4)
#define IPV4_HDR_SIZE               (20)
#define IPV4_HDR_VER                (0x40)

/* IPv4 header manipulation macros. */
#define IPV4_HDR_VER_IHL_OFFSET     0
#define IPV4_HDR_VER_MASK           (0xF0)
#define IPV4_HDR_IHL_MASK           (0xF)
#define IPV4_HDR_DSCP_OFFSET        1
#define IPV4_HDR_LENGTH_OFFSET      2
#define IPV4_HDR_ID_OFFSET          4
#define IPV4_HDR_FLAG_FRAG_OFFSET   6
#define IPV4_HDR_FLAG_MF            (0x2000)
#define IPV4_HDR_FLAG_DF            (0x4000)
#define IPV4_HDR_FRAG_MASK          (0x1FFF)
#define IPV4_HDR_TOL_OFFSET         8
#define IPV4_HDR_PROTO_OFFSET       9
#define IPV4_HDR_CSUM_OFFSET        10
#define IPV4_HDR_SRC_OFFSET         12
#define IPV4_HDR_DST_OFFSET         16
#define IPV4_HDR_OPT_OFFSET         20

/* IPv4 fragment flag definitions. */
#define IPV4_FRAG_IN_USE            0x1
#define IPV4_FRAG_HAVE_FIRST        0x2
#define IPV4_FRAG_LAST_RCVD         0x4
#define IPV4_FRAG_DROP              0x8

/* IPv4 address definitions. */
#define IPV4_ADDR_UNSPEC                (0)
#define IPV4_ADDR_BCAST                 (0xFFFFFFFF)
#define IPV4_ADDR_BCAST_NET(ip, subnet) ((ip) | ((uint32_t)~(subnet)))
#define IPV4_SUBNET_LL                  (0xFFFFFFFF)
#define IPV4_GATWAY_LL                  (0xFFFFFFFF)

#ifdef IPV4_ENABLE_FRAG

/* IPv4 fragment structure. */
typedef struct _ipv4_fragment
{
    /* Networking buffer list for the buffers belong in this fragment. */
    struct _ipv4_fragment_buffer_list
    {
        FS_BUFFER_LIST  *head;
        FS_BUFFER_LIST  *tail;
    } buffer_list;

    /* The system tick at which this fragment will timeout. */
    uint32_t    timeout;

    /* Source address from this fragment is being received. */
    uint32_t    sa;

    /* Packet ID for this fragment list. */
    uint16_t    id;

    /* Fragment flags. */
    uint8_t     flags;
    uint8_t     pad[1];

} IPV4_FRAGMENT;

/* IPv4 fragment data. */
typedef struct _ipv4_fragment_data
{
    /* IPv4 fragment list. */
    IPV4_FRAGMENT   *list;

    /* Number of fragments for this device. */
    uint32_t    num;

    /* Condition data for fragments. */
    CONDITION   condition;
    SUSPEND     suspend;

} IPV4_FRAGMENT_DATA;
#endif /* IPV4_ENABLE_FRAG */

/* IPv4 device data. */
typedef struct _ipv4_device
{
#ifdef IPV4_ENABLE_FRAG
    /* IPv4 fragments for this device. */
    IPV4_FRAGMENT_DATA  fargment;
#endif

    /* IP address assigned to this interface. */
    uint32_t    address;

#ifdef DHCP_CLIENT
    /* IPv4 DHCP client data. */
    DHCP_CLIENT_DEVICE  *dhcp_client;
#endif
} IPV4_DEVICE;

/* Function prototypes. */
void ipv4_device_initialize(NET_DEV *);
uint8_t ipv4_compare_address(uint32_t, uint32_t, uint8_t);
int32_t ipv4_get_device_address(FD, uint32_t *, uint32_t *);
int32_t ipv4_set_device_address(FD, uint32_t, uint32_t);
NET_DEV *ipv4_get_source_device(uint32_t);
int32_t net_process_ipv4(FS_BUFFER_LIST *, uint32_t);
int32_t ipv4_header_add(FS_BUFFER_LIST *, uint8_t, uint32_t, uint32_t, uint8_t);
#ifdef IPV4_ENABLE_FRAG
void ipv4_fragment_set_data(FD, IPV4_FRAGMENT *, uint32_t);
#endif

#endif /* NET_IPV4 */
#endif /* CONFIG_NET */
#endif /* _NET_IPv4_H_ */
