/*
 * net_dhcp_client.h
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
#ifndef _NET_DHCP_CLIENT_H_
#define _NET_DHCP_CLIENT_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef DHCP_CLIENT
#ifndef NET_DHCP
#error "DHCP required for DHCP client."
#endif
#include <net_dhcp.h>
#include <net_udp.h>
#include <net_dhcp_client_config.h>

/* DHCP client states. */
#define DHCP_CLI_DISCOVER       (0x0)
#define DHCP_CLI_REQUEST        (0x1)
#define DHCP_CLI_RENEW          (0x2)
#define DHCP_CLI_REBIND         (0x3)
#define DHCP_CLI_STOPPED        (0xFF)

/* DHCP client data. */
typedef struct _dhcp_client_data
{
    /* UDP port associated with this port. */
    UDP_PORT    udp;

    /* Suspend data for this DHCP client. */
    SUSPEND     suspend;

    /* File system parameter to process data for DHCP client. */
    FS_PARAM    fs_param;

} DHCP_CLIENT_DATA;

/* DHCP client device data. */
struct _dhcp_client_device
{
    /* Condition data for this DHCP client. */
    CONDITION   condition;
    SUSPEND     suspend;

    /* Time at which we acquired a lease. */
    uint32_t    lease_start;

    /* Time at which this client started this transaction. */
    uint32_t    start_time;

    /* Life time of the lease. */
    uint32_t    lease_time;

    /* DHCP server IP address. */
    uint32_t    server_ip;

    /* Assigned IP address. */
    uint32_t    client_ip;

    /* Gateway address for this network. */
    uint32_t    gateway_ip;

    /* Transaction ID used by this DHCP client device. */
    uint32_t    xid;

    /* Current timeout for DHCP client. */
    uint32_t    current_timeout;

    /* DHCP client state. */
    uint8_t     state;

    /* Number of retries we have done on this state, not including the discover
     * state. */
    uint8_t     retry;

    /* IP assigned. */
    uint8_t     address_assigned;

    /* Structure padding. */
    uint8_t     pad[1];

};

/* Function prototypes. */
void net_dhcp_client_initialize(void);
void net_dhcp_client_initialize_device(NET_DEV *, DHCP_CLIENT_DEVICE *);
void net_dhcp_client_start(NET_DEV *);
void net_dhcp_client_stop(NET_DEV *);

#endif /* DHCP_CLIENT */

#endif /* CONFIG_NET */
#endif /* _NET_DHCP_CLIENT_H_ */
