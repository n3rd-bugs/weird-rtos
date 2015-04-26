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
#include <net_csum.h>
#include <header.h>

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
    uint16_t src_port, dst_port, length;
#ifdef UDP_CSUM
    uint16_t csum_hdr;
    uint32_t csum;
    FS_BUFFER *csum_buffer;
    HDR_GEN_MACHINE hdr_machine;
    HEADER pseudo_hdr[] =
    {
        {(uint8_t *)&src_addr,          4, FS_BUFFER_PACKED},   /* Source address. */
        {(uint8_t *)&dst_addr,          4, FS_BUFFER_PACKED},   /* Destination address. */
        {(uint8_t []){0},               1, 0},                  /* Zero. */
        {(uint8_t []){IP_PROTO_UDP},    1, 0},                  /* Protocol. */
        {(uint8_t *)&length,            2, FS_BUFFER_PACKED},   /* UDP length. */
    };
#endif

    /* Pull the length of UDP datagram. */
    OS_ASSERT(fs_buffer_pull_offset(buffer, &length, 2, (ihl + UDP_HRD_LEN_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

#ifdef UDP_CSUM
    /* Pull the checksum for UDP datagram. */
    OS_ASSERT(fs_buffer_pull_offset(buffer, &csum_hdr, 2, (ihl + UDP_HRD_CSUM_OFFSET), (FS_BUFFER_INPLACE)) != SUCCESS);

    /* If we can verify the checksum for UDP header. */
    if (csum_hdr != 0)
    {
        /* Allocate a buffer and initialize a pseudo header. */
        csum_buffer = fs_buffer_get(buffer->fd, FS_BUFFER_LIST, 0);

        /* If we have to buffer to compute checksum. */
        if (csum_buffer != NULL)
        {
            /* Initialize header generator machine. */
            header_gen_machine_init(&hdr_machine, &fs_buffer_hdr_push);

            /* Push the ICMP header on the buffer. */
            status = header_generate(&hdr_machine, pseudo_hdr, sizeof(pseudo_hdr)/sizeof(HEADER), csum_buffer);

            /* If pseudo header was successfully generated. */
            if (status == SUCCESS)
            {
                /* Calculate checksum for the pseudo header. */
                csum = net_csum_calculate(csum_buffer, -1, 0);

                /* Calculate and add the checksum for UDP datagram. */
                NET_CSUM_ADD(csum, net_csum_calculate(buffer, -1, ihl));

                /* If we don't have anticipated checksum. */
                if (csum != 0)
                {
                    /* Return an error to the caller. */
                    status = NET_INVALID_CSUM;
                }
            }

            /* Free the pseudo header buffer. */
            fs_buffer_add(buffer->fd, csum_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }
        else
        {
            /* There are no buffers. */
            status = NET_NO_BUFFERS;
        }
    }
#endif

    if (status == SUCCESS)
    {
        /* Peek the UDP ports and length fields of UDP header. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &src_port, 2, (ihl + UDP_HRD_SRC_PORT_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);
        OS_ASSERT(fs_buffer_pull_offset(buffer, &dst_port, 2, (ihl + UDP_HRD_DST_PORT_OFFSET), (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* If UDP header length value is correct. */
        if (buffer->total_length == (ihl + length))
        {
        }
    }

    /* Return status to the caller. */
    return (status);

} /* net_process_udp */

#endif /* NET_UDP */
#endif /* CONFIG_NET */
