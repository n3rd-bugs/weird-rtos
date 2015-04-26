/*
 * net_udp.c
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
#include <os.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_UDP
#include <net_udp.h>

/*
 * net_process_udp
 * @buffer: File system buffer needed to be processed.
 * @ihl: IPv4 header length.
 * @iface_addr: Interface IP address on which this packet was received.
 * @src_addr: Source address from the IP header.
 * @dst_addr: Destination address from the IP header.
 * This function will process an incoming UDP header.
 */
int32_t net_process_udp(FS_BUFFER *buffer, uint32_t ihl, uint32_t iface_addr, uint32_t src_addr, uint32_t dst_addr)
{
    int32_t status = SUCCESS;
    uint16_t src_port, dst_port;

    /* UDP checksum is not supported. */

    /* Peek the UDP ports. */

    /* Return status to the caller. */
    return (status);

} /* net_process_udp */

#endif /* NET_UDP */
#endif /* CONFIG_NET */
