/*
 * net.h
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
#ifndef _NET_H_
#define _NET_H_
#include <os.h>

#ifdef CONFIG_NET

/* Networking configuration. */
#define NET_IPV4
#define NET_ICMP
#define NET_UDP
#define NET_ARP

#include <net_buffer.h>
#include <net_device.h>
#include <net_process.h>

/* Status code definitions. */
#define NET_BUFFER_CONSUMED     -1000
#define NET_NOT_SUPPORTED       -1001
#define NET_UNKNOWN_PROTO       -1002
#define NET_UNKNOWN_SRC         -1003
#define NET_NO_ACTION           -1004
#define NET_DST_UNREACHABLE     -1005
#define NET_DST_PRT_UNREACHABLE -1006
#define NET_NO_BUFFERS          -1007
#define NET_THRESHOLD           -1008
#define NET_LINK_DOWN           -1009
#define NET_INVALID_CSUM        -1010
#define NET_INVALID_HDR         -1011
#define NET_INVALID_FD          -1012

/* Networking port definitions. */
#define NET_PORT_UNSPEC         0

/* Socket structure. */
typedef struct _socket_address
{
#ifdef NET_IPV4
    /* IPv4 data for this socket. */
    uint32_t    local_ip;
    uint32_t    foreign_ip;
#endif

    /* Port data for this socket. */
    uint16_t    local_port;
    uint16_t    foreign_port;
} SOCKET_ADDRESS;

/* Function prototypes. */
void net_init();
uint8_t net_port_match(uint16_t, uint16_t, uint8_t);
uint8_t net_socket_address_match(SOCKET_ADDRESS *, SOCKET_ADDRESS *);

#endif /* CONFIG_NET */
#endif /* _NET_H_ */
