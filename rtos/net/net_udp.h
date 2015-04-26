/*
 * net_udp.h
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
#ifndef _NET_UDP_H_
#define _NET_UDP_H_
#include <os.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_UDP
#define UDP_CSUM

/* UDP header parser definitions. */
#define UDP_HRD_LENGTH              (8)
#define UDP_HRD_SRC_PORT_OFFSET     0
#define UDP_HRD_DST_PORT_OFFSET     2
#define UDP_HRD_LEN_OFFSET          4
#define UDP_HRD_CSUM_OFFSET         6

/* Function prototypes. */
int32_t net_process_udp(FS_BUFFER *, uint32_t, uint32_t, uint32_t, uint32_t);

#endif /* NET_UDP */
#endif /* CONFIG_NET */
#endif /* _NET_UDP_H_ */
