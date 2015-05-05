/*
 * net_icmp.c
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

#ifdef NET_ICMP
#include <fs.h>
#include <fs_buffer.h>
#include <header.h>
#include <net_icmp.h>
#include <net_csum.h>
#ifdef NET_IPV4
#include <net_ipv4.h>
#else
#error "IPv4 stack required for ICMP."
#endif

/*
 * net_process_icmp
 * @buffer: File system buffer needed to be processed.
 * @ihl: IPv4 header length.
 * @iface_addr: Interface IP address on which this packet was received.
 * @src_addr: Source address from the IP header.
 * @dst_addr: Destination address from the IP header.
 * @return: A success status will be returned if packet was successfully
 *  processed.
 *  NET_INVALID_CSUM will be returned if an inlaid checksum was parsed.
 * This function will process an incoming ICMP packet.
 */
int32_t net_process_icmp(FS_BUFFER *buffer, uint32_t ihl, uint32_t iface_addr, uint32_t src_addr, uint32_t dst_addr)
{
    int32_t status = SUCCESS;
    uint32_t id_seq;
    uint8_t type;

    /* Calculate and verify the checksum for the ICMP packet. */
    if (net_csum_calculate(buffer, -1, ihl) == 0)
    {
        /* Peek the type of the ICMP packet. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &type, 1, ihl + ICMP_HDR_TYPE_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

#ifdef ICMP_ENABLE_PING
        /* If this is an Echo request. */
        if (type == ICMP_ECHO_REQUEST)
        {
            /* Process ICMP echo request. */

            /* If we are intended destination. */
            if (iface_addr == dst_addr)
            {
                /* Pull the ID and sequence number. */
                OS_ASSERT(fs_buffer_pull_offset(buffer, &id_seq, 4, ihl + ICMP_HDR_TRAIL_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

                /* Pull the IP and ICMP packet header header. */
                OS_ASSERT(fs_buffer_pull_offset(buffer, NULL, ihl + ICMP_HDR_PAYLOAD_OFFSET, 0, 0) != SUCCESS);

                /* Initialize an ICMP Echo reply. */
                status = icmp_header_add(buffer, ICMP_ECHO_REPLY, 0, id_seq);

                if (status == SUCCESS)
                {
                    /* Add IPv4 header on the packet. */
                    status = ipv4_header_add(buffer, IP_PROTO_ICMP, iface_addr, src_addr, 0);
                }

                if (status == SUCCESS)
                {
                    /* Transmit an ICMP packet. */
                    status = net_device_buffer_transmit(buffer, NET_PROTO_IPV4, 0);
                }
            }
        }
#endif /* ICMP_ENABLE_PING */
    }
    else
    {
        /* Invalid checksum was received. */
        status = NET_INVALID_CSUM;
    }

    /* Return status to the caller. */
    return (status);

} /* net_process_icmp */

/*
 * icmp_header_add
 * @buffer: File system buffer on which ICMP header is needed to be added.
 * @type: ICMP type.
 * @code: ICMP type code.
 * @unused: Unused bytes.
 * @return: A success status will be returned if ICMP header was successfully
 *  added.
 * This function will add an ICMP header on the given buffer.
 */
int32_t icmp_header_add(FS_BUFFER *buffer, uint8_t type, uint8_t code, uint32_t unused)
{
    int32_t status = SUCCESS;
    HDR_GEN_MACHINE hdr_machine;
    uint16_t csum = 0;
    HEADER headers[] =
    {
        {&type,     1, 0},  /* ICMP type. */
        {&code,     1, 0},  /* ICMP code. */
        {&csum,     2, 0},  /* Checksum. */
        {&unused,   4, 0},  /* Unused data. */
    };

    /* Push the ICMP header. */

    /* We already have the payload in the buffer. */

    /* Initialize header generator machine. */
    header_gen_machine_init(&hdr_machine, &fs_buffer_hdr_push);

    /* Push the ICMP header on the buffer. */
    status = header_generate(&hdr_machine, headers, sizeof(headers)/sizeof(HEADER), buffer);

    if (status == SUCCESS)
    {
        /* Compute and update the value of checksum field. */
        csum = net_csum_calculate(buffer, -1, 0);
        status = fs_buffer_push_offset(buffer, &csum, 2, ICMP_HDR_CSUM_OFFSET, (FS_BUFFER_HEAD | FS_BUFFER_UPDATE));
    }

    /* Return status to the caller. */
    return (status);

} /* icmp_header_add */

#endif /* NET_ICMP */
#endif /* CONFIG_NET */
