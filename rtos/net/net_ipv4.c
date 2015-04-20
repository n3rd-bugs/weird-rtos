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
#include <net_csum.h>

/*
 * ipv4_get_device_address
 * @fd: File descriptor associated with the networking device.
 * @address: IPv4 address associated with this device will be returned here.
 * @return: A success status will be returned if IPv4 address was successfully
 *  returned, NET_INVALID_FD will be returned if the given file descriptor don't
 *  have an associated networking device.
 * This function will return IPv4 address associated with the networking device
 * as resolved from the file descriptor.
 */
int32_t ipv4_get_device_address(FD fd, uint32_t *address)
{
    NET_DEV *net_device = net_device_get_fd(fd);
    int32_t status = SUCCESS;

    if (net_device != NULL)
    {
#ifdef CONFIG_SEMAPHORE
        /* Obtain the lock for this networking device. */
        OS_ASSERT(semaphore_obtain(&net_device->lock, MAX_WAIT) != SUCCESS);
#else
        /* Lock the scheduler. */
        scheduler_lock();
#endif

        /* Return the IPv4 address assigned to this device. */
        *address = net_device->ipv4_address;

#ifndef CONFIG_SEMAPHORE
        /* Enable scheduling. */
        scheduler_unlock();
#else
        /* Release the lock for this networking device. */
        semaphore_release(&net_device->lock);
#endif
    }
    else
    {
        /* We did not find a valid networking device for given file descriptor. */
        status = NET_INVALID_FD;
    }

    /* Return status to the caller. */
    return (status);

} /* ipv4_get_device_address */

/*
 * ipv4_set_device_address
 * @fd: File descriptor associated with the networking device.
 * @address: IPv4 address needed to be updated.
 * @return: A success status will be returned if IPv4 address was successfully
 *  returned, NET_INVALID_FD will be returned if the given file descriptor don't
 *  have an associated networking device.
 * This function will update the IPv4 address associated with the networking
 * device as resolved from the file descriptor.
 */
int32_t ipv4_set_device_address(FD fd, uint32_t address)
{
    NET_DEV *net_device = net_device_get_fd(fd);
    int32_t status = SUCCESS;

    if (net_device != NULL)
    {
#ifdef CONFIG_SEMAPHORE
        /* Obtain the lock for this networking device. */
        OS_ASSERT(semaphore_obtain(&net_device->lock, MAX_WAIT) != SUCCESS);
#else
        /* Lock the scheduler. */
        scheduler_lock();
#endif

        /* Update the IPv4 address assigned to this device. */
        net_device->ipv4_address = address;

#ifndef CONFIG_SEMAPHORE
        /* Enable scheduling. */
        scheduler_unlock();
#else
        /* Release the lock for this networking device. */
        semaphore_release(&net_device->lock);
#endif
    }
    else
    {
        /* We did not find a valid networking device for given file descriptor. */
        status = NET_INVALID_FD;
    }

    /* Return status to the caller. */
    return (status);

} /* ipv4_set_device_address */

/*
 * net_process_ipv4
 * @buffer: File system buffer needed to be processed.
 * This function will process an incoming IPv4 packet.
 */
int32_t net_process_ipv4(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    uint32_t sa = 0, da;
    uint8_t proto, keep, ver_ihl, icmp_rep;

    /* We must have at least one byte to verify an IPv4 packet. */
    if (buffer->total_length >= 1)
    {
        /* Peek the version and IHL. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &ver_ihl, 1, IPV4_HDR_VER_IHL_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

        /* Check if we have a valid IPv4 version. */
        if ((ver_ihl & IPV4_HDR_VER_MASK) == IPV4_HDR_VER)
        {
            /* Calculate the internet header length. */

            /* Internet Header Length is the length of the internet header in
             * 32-bit words. */
            ver_ihl = (uint8_t)((ver_ihl & IPV4_HDR_IHL_MASK) << 2);

            /* If we don't have anticipated number of bytes on the buffer. */
            if (buffer->total_length < ver_ihl)
            {
                /* Return an error. */
                status = NET_INVALID_HDR;
            }
        }
        else
        {
            /* Return an error. */
            status = NET_INVALID_HDR;
        }
    }
    else
    {
        /* Return an error. */
        status = NET_INVALID_HDR;
    }

    if (status == SUCCESS)
    {
        /* With a valid checksum the recalculation of checksum should return 0. */
        if (net_csum_calculate(buffer, ver_ihl) != 0)
        {
            /* Return an error. */
            status = NET_INVALID_HDR;
        }
    }

    if (status == SUCCESS)
    {
        /* Peek the IPv4 protocol and see if we do support it. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &proto, 1, IPV4_HDR_PROTO_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

#ifdef NET_ICMP
        /* Try to resolve the protocol to which this packet is needed to be
         * forwarded. */
        if (proto == IP_PROTO_ICMP)
        {
            /* Process ICMP packet. */
            status = net_process_icmp(buffer, ver_ihl);
        }

        /* Protocol was not resolved. */
        else
#endif

        {
#ifdef NET_ICMP
            /* Pick the address to which this packet was addressed to. */
            OS_ASSERT(fs_buffer_pull_offset(buffer, &da, 4, IPV4_HDR_DST_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

            /* Get IPv4 address assigned to this device. */
            OS_ASSERT(ipv4_get_device_address(buffer->fd, &sa) != SUCCESS);

            /* If this packet for intended for us. */
            if (sa == da)
            {
                /* Protocol not resolved. */
                icmp_rep = ICMP_DST_PROTO;
            }
            else
            {
                /* Cannot forward this packet, destination unreachable. */
                icmp_rep = ICMP_DST_HOST;
            }

            /* Pick the address to which we will be sending unreachable message. */
            OS_ASSERT(fs_buffer_pull_offset(buffer, &da, 4, IPV4_HDR_SRC_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

            /* The internet header plus the first 64 bits of the original
             * datagram's data.  This data is used by the host to match the
             * message to the appropriate process.  If a higher level protocol
             * uses port numbers, they are assumed to be in the first 64 data
             * bits of the original datagram's data. */
            keep = (uint8_t)(ver_ihl + 8);

            /* Check if we don't have 64 bits ahead. */
            if (keep < buffer->total_length)
            {
                /* Pull the data that is not needed to be sent. */
                OS_ASSERT(fs_buffer_pull(buffer, NULL, (buffer->total_length - keep), FS_BUFFER_TAIL) != SUCCESS);
            }

            /* Generate an ICMP protocol unreachable message. */
            status = icmp_header_add(buffer, ICMP_DST_UNREACHABLE, icmp_rep, 0);

            if (status == SUCCESS)
            {
                /* Add IPv4 packet on the packet. */
                status = ipv4_header_add(buffer, IP_PROTO_ICMP, sa, da);
            }

            if (status == SUCCESS)
            {
                /* Transmit an IPv4 packet. */
                status = net_device_buffer_transmit(buffer, NET_PROTO_IPV4);
            }

            if (status == SUCCESS)
            {
                /* We have transmitted the same buffer. */
                status = NET_BUFFER_CONSUMED;
            }
#endif
        }
    }

    /* Return status to the caller. */
    return (status);

} /* net_process_ipv4 */

/*
 * ipv4_header_add
 * @buffer: File system buffer on which IPv4 header is needed to be added.
 * This function will add an IPv4 header on the given buffer.
 */
int32_t ipv4_header_add(FS_BUFFER *buffer, uint8_t proto, uint32_t src_addr, uint32_t dst_addr)
{
    int32_t status = SUCCESS;
    HDR_GEN_MACHINE hdr_machine;
    uint8_t ver_ihl, ihl = ALLIGN_CEIL_N((IPV4_HDR_SIZE), 4) / 4, dscp = 0, ttl = 128;
    uint16_t id = 0, flag_offset = 0, csum = 0;
    uint16_t total_length = (uint16_t)(buffer->total_length + (uint32_t)(ihl * 4));
    HEADER headers[] =
    {
        {&ver_ihl,      1,  0 },                    /* Version and IHL. */
        {&dscp,         1,  0 },                    /* DSCP. */
        {&total_length, 2,  FS_BUFFER_PACKED },     /* Total length. */
        {&id,           2,  0 },                    /* Fragment ID. */
        {&flag_offset,  2,  0 },                    /* Flags and fragment offset. */
        {&ttl,          1,  0 },                    /* Time to live. */
        {&proto,        1,  0 },                    /* Protocol. */
        {&csum,         2,  0 },                    /* Checksum. */
        {&src_addr,     4,  FS_BUFFER_PACKED },     /* Source address. */
        {&dst_addr,     4,  FS_BUFFER_PACKED },     /* Destination address. */
    };

    /* A packet size must not exceed this. */
    OS_ASSERT(buffer->total_length > (65535 - IPV4_HDR_SIZE));

    /* Create the version and header length headers. */
    ver_ihl = (ihl | IPV4_HDR_VER);

    /* If we need to add IPv4 header options. */
    if (ihl > (IPV4_HDR_SIZE / 4))
    {
        /* [TODO] Add IPv4 header options. */
    }

    /* Initialize header generator machine. */
    header_gen_machine_init(&hdr_machine, &fs_buffer_hdr_push);

    /* Push the IPv4 header on the buffer. */
    status = header_generate(&hdr_machine, headers, sizeof(headers)/sizeof(HEADER), buffer);

    if (status == SUCCESS)
    {
        /* Compute and update the value of checksum field. */
        csum = net_csum_calculate(buffer, (ihl << 2));
        status = fs_buffer_push_offset(buffer, &csum, 2, IPV4_HDR_CSUM_OFFSET, (FS_BUFFER_HEAD | FS_BUFFER_UPDATE));
    }

    /* Return status to the caller. */
    return (status);

} /* ipv4_header_add */

#endif /* NET_IPV4 */
#endif /* CONFIG_NET */
