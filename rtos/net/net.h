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

#include <net_buffer.h>
#include <net_device.h>
#include <net_process.h>

/* Status code definitions. */
#define NET_BUFFER_CONSUMED     -1000
#define NET_NOT_SUPPORTED       -1001
#define NET_UNKNOWN_PROTO       -1002
#define NET_DEST_UNREACHABLE    -1003
#define NET_INVALID_HDR         -1004
#define NET_INVALID_FD          -1005

/* Function prototypes. */
void net_init();

#endif /* CONFIG_NET */
#endif /* _NET_H_ */
