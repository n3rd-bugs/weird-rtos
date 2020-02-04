/*
 * net_route.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _NET_ROUTE_H_
#define _NET_ROUTE_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <net.h>
#ifdef NET_IPV4
#include <net_route_config.h>

/* Error definitions. */
#define ROUTE_EXIST             -1400
#define ROUTE_NO_MEM            -1401

/* Route flag definitions. */
#define ROUTE_VALID             (0x1)

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
void route_print(void);

#endif /* NET_IPV4 */
#endif /* CONFIG_NET */
#endif /* _NET_ROUTE_H_ */
