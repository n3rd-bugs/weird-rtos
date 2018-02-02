/*
 * net.h
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
#ifndef _NET_H_
#define _NET_H_
#include <kernel.h>

#ifdef CONFIG_NET

/* Networking configuration. */
#ifdef CMAKE_BUILD
#include <net_config.h>
#else
#define NET_IPV4
#define NET_ICMP
#define NET_UDP
#define NET_TCP
#define NET_ARP
#define NET_DHCP
#endif /* CMAKE_BUILD */

#include <net_buffer.h>
#include <net_device.h>
#include <net_process.h>

/* Status code definitions. */
#define NET_BUFFER_CONSUMED     -1000
#define NET_NOT_SUPPORTED       -1001
#define NET_UNKNOWN_PROTO       -1002
#define NET_UNKNOWN_SRC         -1003
#define NET_NO_ACTION           -1004
#define NET_REFUSED             -1005
#define NET_CLOSED              -1006
#define NET_DST_UNREACHABLE     -1007
#define NET_DST_PRT_UNREACHABLE -1008
#define NET_NO_NEXT_OPT         -1009
#define NET_THRESHOLD           -1010
#define NET_LINK_DOWN           -1011
#define NET_INVALID_CSUM        -1012
#define NET_INVALID_HDR         -1013
#define NET_INVALID_FD          -1014
#define NET_NO_RTX_AVAILABLE    -1015

/* Networking port definitions. */
#define NET_PORT_UNSPEC         0

/* Networking priority configurations. */
#define NET_DEVICE_PRIORITY     (0)
#define NET_DEMUX_PRIORITY      (1)
#define NET_ARP_PRIORITY        (2)
#define NET_IPV4_PRIORITY       (3)
#define NET_SOCKET_PRIORITY     (4)
#define NET_USER_PRIORITY       (SUSPEND_MIN_PRIORITY)

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
void net_init(void);
uint16_t net_port_random(void);
uint8_t net_port_match(uint16_t, uint16_t, uint8_t);
uint8_t net_socket_address_match(SOCKET_ADDRESS *, SOCKET_ADDRESS *);

#endif /* CONFIG_NET */
#endif /* _NET_H_ */
