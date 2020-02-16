/*
 * net.c
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
#include <kernel.h>

#ifdef CONFIG_NET
#include <net.h>
#include <net_condition.h>
#ifdef NET_TCP
#include <net_tcp.h>
#endif
#ifdef NET_UDP
#include <net_udp.h>
#endif
#ifdef IO_ETHERNET
#include <ethernet.h>
#endif
#ifdef NET_DHCP
#include <net_dhcp.h>
#endif
#ifdef DHCP_CLIENT
#include <net_dhcp_client.h>
#endif

/*
 * net_init
 * This function will initialize the networking stack.
 */
void net_init(void)
{
    SYS_LOG_FUNCTION_ENTRY(NET);

    /* Initialize networking devices. */
    net_devices_init();

    /* Initialize networking buffers file system. */
    net_buffer_init();

    /* Initialize networking conditions. */
    net_condition_init();

#ifdef NET_TCP
    /* Initialize TCP stack. */
    tcp_initialize();
#endif

#ifdef NET_UDP
    /* Initialize UDP stack. */
    udp_initialize();
#endif

#ifdef DHCP_CLIENT
    /* Initialize DHCP client stack. */
    net_dhcp_client_initialize();
#endif

#ifdef IO_ETHERNET
    /* Initialize ethernet devices. */
    ethernet_init();
#endif

    SYS_LOG_FUNCTION_EXIT(NET);

} /* net_buffer_init */

/*
 * net_port_random
 * @return: A random 16-bit port number will be returned here.
 * This function will generate a random 16-bit port number and return it's
 * value.
 */
uint16_t net_port_random(void)
{
    /* Return a random port number. */
    return ((uint16_t)(current_hardware_tick() & 0xFFFF));

} /* net_port_random */

/*
 * net_port_match
 * @port1: Port needed to be matched.
 * @port2: Given port.
 * @return: A true will be returned if ports match exactly, partial will be
 *  returned if they match partially and false will be returned if ports don't
 *  match at all.
 * This function will match two given networking ports.
 */
uint8_t net_port_match(uint16_t port1, uint16_t port2, uint8_t match)
{
    SYS_LOG_FUNCTION_ENTRY(NET);

    /* If match is not already failed. */
    if (match != FALSE)
    {
        /* Compare the two networking ports. */

        /* If we don't have an unspecified port. */
        if (port1 != NET_PORT_UNSPEC)
        {
            /* If the address exactly match. */
            if (port1 == port2)
            {
                /* A partial match cannot be updated to exact match. */
                if (match != PARTIAL)
                {
                    /* Exact match. */
                    match = TRUE;
                }
            }
            else
            {
                /* Did not match at all. */
                match = FALSE;
            }
        }
        else
        {
            /* Got a partial match. */
            match = PARTIAL;
        }
    }

    SYS_LOG_FUNCTION_EXIT(NET);

    /* Return the match status to the caller. */
    return (match);

} /* net_port_match */

/*
 * net_socket_address_match
 * @socket1: Socket needed to be matched.
 * @socket2: Given socket address.
 * @return: A true will be returned if sockets match exactly, partial will be
 *  returned if they match partially and false will be returned if sockets don't
 *  match at all.
 * This function will match two given networking sockets.
 */
uint8_t net_socket_address_match(SOCKET_ADDRESS *socket1, SOCKET_ADDRESS *socket2)
{
    uint8_t match;

    SYS_LOG_FUNCTION_ENTRY(NET);

#ifdef NET_IPV4
    /* Compare the local and foreign IP addresses. */
    match = ipv4_compare_address(socket1->local_ip, socket2->local_ip, TRUE);
    match = ipv4_compare_address(socket1->foreign_ip, socket2->foreign_ip, match);
#endif

    /* Compare the local and foreign ports. */
    match = net_port_match(socket1->local_port, socket2->local_port, TRUE);
    match = net_port_match(socket1->foreign_port, socket2->foreign_port, match);

    SYS_LOG_FUNCTION_EXIT(NET);

    /* Return the match status to the caller. */
    return (match);

} /* net_socket_address_match */

#endif /* CONFIG_NET */
