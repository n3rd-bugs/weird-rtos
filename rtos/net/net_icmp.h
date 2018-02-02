/*
 * net_icmp.h
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
#ifndef _NET_ICMP_H_
#define _NET_ICMP_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_ICMP

/* ICMP configuration. */
#ifdef CMAKE_BUILD
#include <net_icmp_config.h>
#else
#define ICMP_ENABLE_PING
#endif /* CMAKE_BUILD */

/* ICMP definitions. */
#define ICMP_ECHO_REPLY         0
#define ICMP_DST_UNREACHABLE    3
#define ICMP_SRC_QUENCH         4
#define ICMP_REDIRECT           5
#define ICMP_ECHO_REQUEST       8

/* ICMP destination unreachable codes. */
#define ICMP_DST_NET            0x00
#define ICMP_DST_HOST           0x01
#define ICMP_DST_PROTO          0x02
#define ICMP_DST_PORT           0x03
#define ICMP_DST_DF_SET         0x04
#define ICMP_DST_ROUTE          0x05
#define ICMP_DST_NONE           0xFF

/* ICMP header manipulation definitions. */
#define ICMP_HDR_TYPE_OFFSET    0
#define ICMP_HDR_CODE_OFFSET    1
#define ICMP_HDR_CSUM_OFFSET    2
#define ICMP_HDR_TRAIL_OFFSET   4
#define ICMP_HDR_PAYLOAD_OFFSET 8

/* Function prototypes. */
int32_t net_process_icmp(FS_BUFFER_LIST *, uint32_t, uint32_t, uint32_t, uint32_t);
int32_t icmp_header_add(FS_BUFFER_LIST *, uint8_t, uint8_t, uint32_t);

#endif /* NET_ICMP */
#endif /* CONFIG_NET */
#endif /* _NET_ICMP_H_ */
