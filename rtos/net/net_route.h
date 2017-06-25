/*
 * net_route.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#ifndef _NET_ROUTE_H_
#define _NET_ROUTE_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <net.h>
#ifdef NET_IPV4

/* Number of route entries to manage. */
#define NET_NUM_ROUTES          (4)

/* Error definitions. */
#define ROUTE_EXIST             -1400
#define ROUTE_NO_MEM            -1401

/* Route flag definitions. */
#define ROUTE_VALID             (0x01)

/* A single route entry. */
typedef struct _net_route
{
    /* Associated device for this route. */
    FD          *device;

    /* Gateway address of route. */
    uint32_t    gateway;

    /* Destination address. */
    uint32_t    destination;

    /* Subnet mask. */
    uint32_t    subnet;

    /* Interface address. */
    uint32_t    interface_address;

    /* Route flags. */
    uint8_t    flags;

    /* Structure padding. */
    uint8_t     pad[3];

} NET_ROUTE;

/* Function prototypes. */
int32_t route_add(FD, uint32_t, uint32_t, uint32_t, uint32_t, uint8_t);
int32_t route_remove(FD, uint32_t, uint32_t);
int32_t route_get(FD *, uint32_t, uint32_t *, uint32_t *, uint32_t *);
void route_print();

#endif /* NET_IPV4 */
#endif /* CONFIG_NET */
#endif /* _NET_ROUTE_H_ */
