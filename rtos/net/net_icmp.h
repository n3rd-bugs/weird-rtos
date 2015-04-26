/*
 * net_icmp.h
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
#ifndef _NET_ICMP_H_
#define _NET_ICMP_H_
#include <os.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_ICMP

/* ICMP configuration. */
#define ICMP_ENABLE_PING

/* ICMP definitions. */
#define ICMP_ECHO_REPLY         0
#define ICMP_DST_UNREACHABLE    3
#define ICMP_SRC_QUENCH         4
#define ICMP_REDIRECT           5
#define ICMP_ECHO_REQUEST       8

/* ICMP destination unreachable codes. */
#define ICMP_DST_NET            0
#define ICMP_DST_HOST           1
#define ICMP_DST_PROTO          2
#define ICMP_DST_PORT           3
#define ICMP_DST_DF_SET         4
#define ICMP_DST_ROUTE          5

/* ICMP header manipulation definitions. */
#define ICMP_HDR_TYPE_OFFSET    0
#define ICMP_HDR_CODE_OFFSET    1
#define ICMP_HDR_CSUM_OFFSET    2
#define ICMP_HDR_TRAIL_OFFSET   4
#define ICMP_HDR_PAYLOAD_OFFSET 8

/* Function prototypes. */
int32_t net_process_icmp(FS_BUFFER *, uint32_t, uint32_t, uint32_t, uint32_t);
int32_t icmp_header_add(FS_BUFFER *, uint8_t, uint8_t, uint32_t);

#endif /* NET_ICMP */
#endif /* CONFIG_NET */
#endif /* _NET_ICMP_H_ */
