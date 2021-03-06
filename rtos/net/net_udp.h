/*
 * net_udp.h
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
#ifndef _NET_UDP_H_
#define _NET_UDP_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_UDP
#include <console.h>
#include <net_udp_config.h>

/* UDP header definitions. */
#define UDP_HRD_LENGTH              (8)
#define UDP_HRD_SRC_PORT_OFFSET     (0)
#define UDP_HRD_DST_PORT_OFFSET     (2)
#define UDP_HRD_LEN_OFFSET          (4)
#define UDP_HRD_CSUM_OFFSET         (6)

/* UDP port flags. */
#define UDP_FLAG_THR_BUFFERS        (0x1)

/* UDP port structure. */
typedef struct _udp_port UDP_PORT;
struct _udp_port
{
    /* Console structure for this UDP port. */
    CONSOLE         console;

    /* UDP buffer lists. */
    struct _udp_port_buffer_list
    {
        FS_BUFFER_LIST  *head;
        FS_BUFFER_LIST  *tail;
    } buffer_list;

    /* UDP port list member. */
    UDP_PORT        *next;

    /* UDP socket address. */
    SOCKET_ADDRESS  socket_address;

    /* Destination address. */
    SOCKET_ADDRESS  destination_address;

    /* Last datagram address. */
    SOCKET_ADDRESS  last_datagram_address;

    /* UDP port flags. */
    uint8_t         flags;

    /* Padding variable. */
    uint8_t         pad[7];

};

/* UDP global data. */
typedef struct _udp_data
{
    /* Port list. */
    struct _udp_data_port_list
    {
        UDP_PORT    *head;
        UDP_PORT    *tail;
    } port_list;

#ifdef CONFIG_SEMAPHORE
    /* Data lock to protect global UDP data. */
    SEMAPHORE   lock;
#endif

} UDP_DATA;

/* UDP port search parameter. */
typedef struct _udp_port_param
{
    /* Resolved UDP port. */
    UDP_PORT        *port;

    /* Socket search data. */
    SOCKET_ADDRESS  socket_address;
} UDP_PORT_PARAM;

/* Function prototypes. */
void udp_initialize(void);
void udp_register(UDP_PORT *, char *, SOCKET_ADDRESS *);
void udp_unregister(UDP_PORT *);
int32_t net_process_udp(FS_BUFFER_LIST *, uint32_t, uint32_t, uint32_t, uint32_t);
int32_t udp_header_add(FS_BUFFER_LIST *, SOCKET_ADDRESS *, uint8_t);

#endif /* NET_UDP */
#endif /* CONFIG_NET */
#endif /* _NET_UDP_H_ */
