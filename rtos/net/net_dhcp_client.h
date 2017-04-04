/*
 * net_dhcp_client.h
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
#ifndef _NET_DHCP_CLIENT_H_
#define _NET_DHCP_CLIENT_H_
#include <os.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef DHCP_CLIENT
#ifndef NET_DHCP
#error "DHCP required for DHCP client."
#endif
#include <net_dhcp.h>
#include <net_udp.h>

/* DHCP client configuration. */
#define DHCP_BASE_TIMEOUT       (2 * OS_TICKS_PER_SEC)
#define DHCP_MAX_TIMEOUT        (64 * OS_TICKS_PER_SEC)
#define DHCP_MAX_RETRY          (4)
#define DHCP_CLIENT_HOSTNAME    "weird-rtos"

/* DHCP client states. */
#define DHCP_CLI_DISCOVER       (0x00)
#define DHCP_CLI_REQUEST        (0x01)
#define DHCP_CLI_RENEW          (0x02)
#define DHCP_CLI_REBIND         (0x03)
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

    /* Structure padding. */
    uint8_t     pad[4];

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
    uint16_t    current_timeout;

    /* DHCP client state. */
    uint8_t     state;

    /* Number of retries we have done on this state, not including the discover
     * state. */
    uint8_t     retry;

    /* IP assigned. */
    uint8_t     address_assigned;

    /* Structure padding. */
    uint8_t     pad[3];

};

/* Function prototypes. */
void net_dhcp_client_initialize();
void net_dhcp_client_initialize_device(NET_DEV *, DHCP_CLIENT_DEVICE *);
void net_dhcp_client_start(NET_DEV *);
void net_dhcp_client_stop(NET_DEV *);

#endif /* DHCP_CLIENT */

#endif /* CONFIG_NET */
#endif /* _NET_DHCP_CLIENT_H_ */
