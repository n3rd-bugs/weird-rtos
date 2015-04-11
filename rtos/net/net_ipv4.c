/*
 * net_ipv4.c
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
#include <fs.h>
#include <header.h>
#include <net.h>

#ifdef NET_IPV4
#include <net_ipv4.h>
#ifdef NET_ICMP
#include <net_icmp.h>
#endif

/*
 * net_process_ipv4
 * @buffer: File system buffer needed to be processed.
 * This function will process an incoming IPv4 packet.
 */
int32_t net_process_ipv4(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    uint8_t proto, keep;

    /* Verify that we do have a valid IPv4 packet. */
    if (buffer->total_length >= IPV4_HDR_SIZE)
    {
        /* Peek the IPv4 protocol and see if we do support it. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &proto, 1, IPV4_HDR_PROTO_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

        /* Try to resolve the protocol to which this packet is needed to be
         * forwarded. */

        /* Protocol was not resolved. */
        {
#ifdef NET_ICMP
            /* The internet header plus the first 64 bits of the original
             * datagram's data.  This data is used by the host to match the
             * message to the appropriate process.  If a higher level protocol
             * uses port numbers, they are assumed to be in the first 64 data
             * bits of the original datagram's data. */
            keep = IPV4_HDR_SIZE + 8;

            /* Check if we don't have 64 bits ahead. */
            if (keep < buffer->total_length)
            {
                /* Pull the data that is not needed to be sent. */
                OS_ASSERT(fs_buffer_pull(buffer, NULL, (buffer->total_length - keep), FS_BUFFER_TAIL) != SUCCESS);
            }

            /* Generate an ICMP protocol unreachable message. */
            status = icmp_header_add(buffer, ICMP_DST_UNREACHABLE, ICMP_DST_PROTO);
#endif
        }
    }
    else
    {
        /* This is an invalid packet. */
        status = NET_INVALID_HDR;
    }

    /* Return status to the caller. */
    return (status);

} /* net_process_ipv4 */

/*
 * ipv4_header_add
 * @buffer: File system buffer on which IPv4 header is needed to be added.
 * This function will add an IPv4 header on the given buffer.
 */
int32_t ipv4_header_add(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;

    /* Return status to the caller. */
    return (status);

} /* ipv4_header_add */

#endif /* NET_IPV4 */
#endif /* CONFIG_NET */
