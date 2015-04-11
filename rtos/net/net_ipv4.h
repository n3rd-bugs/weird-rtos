/*
 * net_ipv4.h
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
#ifndef _NET_IPv4_H_
#define _NET_IPv4_H_
#include <os.h>

#ifdef CONFIG_NET
#ifdef NET_IPV4

/* IPv4 header definitions. */
#define IPV4_HDR_SIZE               24

/* IPv4 header manipulation macros. */
#define IPV4_HDR_VER_IHL_OFFSET     0
#define IPV4_HDR_TOS_OFFSET         1
#define IPV4_HDR_LENGTH_OFFSET      2
#define IPV4_HDR_ID_OFFSET          4
#define IPV4_HDR_FLAG_FRAG_OFFSET   6
#define IPV4_HDR_TOL_OFFSET         8
#define IPV4_HDR_PROTO_OFFSET       9
#define IPV4_HDR_CSUM_OFFSET        10
#define IPV4_HDR_SRC_OFFSET         12
#define IPV4_HDR_DST_OFFSET         16
#define IPV4_HDR_OPT_OFFSET         20

/* Function prototypes. */
int32_t net_process_ipv4(FS_BUFFER *);
int32_t ipv4_header_add(FS_BUFFER *);

#endif /* NET_IPV4 */
#endif /* CONFIG_NET */
#endif /* _NET_IPv4_H_ */
