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
 * This function will process an incoming ICMP packet.
 */
int32_t net_process_icmp(FS_BUFFER *buffer, uint32_t ihl)
{
    int32_t status = SUCCESS;
    uint32_t sa, da, id_seq;
    uint8_t type;

    /* Peek the type of the ICMP packet. */
    OS_ASSERT(fs_buffer_pull_offset(buffer, &type, 1, (ihl + ICMP_HDR_TYPE_OFFSET), FS_BUFFER_INPLACE) != SUCCESS);

    /* If this is an Echo request. */
    if (type == ICMP_ECHO_REQUEST)
    {
        /* Process ICMP echo request. */

        /* Pull the IPv4 source and destination address. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &da, 4, IPV4_HDR_DST_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* Pull the IPv4 address assigned to this device. */
        OS_ASSERT(ipv4_get_device_address(buffer->fd, &sa) != SUCCESS);

        /* If we are intended destination. */
        if (da == sa)
        {
            /* Pull the source address to which we will be sending the reply. */
            OS_ASSERT(fs_buffer_pull_offset(buffer, &da, 4, IPV4_HDR_SRC_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

            /* Pull the ID and sequence number. */
            OS_ASSERT(fs_buffer_pull_offset(buffer, &id_seq, 4, (ihl + ICMP_HDR_TRAIL_OFFSET), FS_BUFFER_INPLACE) != SUCCESS)

            /* Pull the IPv4 header and the ICMP packet header. */
            OS_ASSERT(fs_buffer_pull_offset(buffer, NULL, ihl + ICMP_HDR_PAYLOAD_OFFSET, 0, 0) != SUCCESS);

            /* Initialize an ICMP Echo reply. */
            status = icmp_header_add(buffer, ICMP_ECHO_REPLY, 0, id_seq);

            if (status == SUCCESS)
            {
                /* Add IPv4 packet on the packet. */
                status = ipv4_header_add(buffer, IP_PROTO_ICMP, sa, da);
            }

            if (status == SUCCESS)
            {
                /* Transmit an IPv4 packet. */
                if (net_device_buffer_transmit(buffer, NET_PROTO_IPV4) == SUCCESS)
                {
                    /* We have transmitted the same buffer. */
                    status = NET_BUFFER_CONSUMED;
                }
            }
        }
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
        csum = net_csum_calculate(buffer, -1);
        status = fs_buffer_push_offset(buffer, &csum, 2, ICMP_HDR_CSUM_OFFSET, (FS_BUFFER_HEAD | FS_BUFFER_UPDATE));
    }

    /* Return status to the caller. */
    return (status);

} /* icmp_header_add */

#endif /* NET_ICMP */
#endif /* CONFIG_NET */
