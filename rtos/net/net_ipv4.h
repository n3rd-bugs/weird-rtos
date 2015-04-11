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

/* IPv4 data structure. */
typedef struct _ipv4_hdr
{
    /* IPv4 source address. */
    uint32_t    src_addr;

    /* IPv4 destination address. */
    uint32_t    dst_addr;

    /* IPv4 total length. */
    uint16_t    total_length;

    /* IPv4 protocol. */
    uint8_t     protocol;

    uint8_t     pad[1];

} IPV4_PKT_DATA;

/* Function prototypes. */
int32_t net_process_ipv4(FS_BUFFER *);

#endif /* NET_IPV4 */
#endif /* CONFIG_NET */
#endif /* _NET_IPv4_H_ */
