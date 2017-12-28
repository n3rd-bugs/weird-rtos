/*
 * net_ipv4.c
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
#include <fs.h>
#include <header.h>
#include <net.h>

#ifdef NET_IPV4
#include <net_ipv4.h>
#ifdef NET_ICMP
#include <net_icmp.h>
#endif
#ifdef NET_TCP
#include <net_tcp.h>
#endif
#ifdef NET_UDP
#include <net_udp.h>
#endif
#include <net_csum.h>
#include <sll.h>
#include <string.h>
#include <net_route.h>

/* Internal function prototypes. */
#ifdef IPV4_ENABLE_FRAG
static void ipv4_fragment_update_timer(NET_DEV *);
static void ipv4_fragment_expired(void *, int32_t);
static uint8_t ipv4_frag_sort(void *, void *);
static int32_t ipv4_frag_add(FS_BUFFER *, uint16_t);
static int32_t ipv4_frag_merge(IPV4_FRAGMENT *, FS_BUFFER *);
#endif

/*
 * ipv4_device_initialize
 * @net_dev: Networking device for which IPv4 data is needed to be initialized.
 * This function will initialize IPv4 data for a new networking device.
 */
void ipv4_device_initialize(NET_DEV *net_dev)
{
    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* Clear the IPv4 address assigned to this device. */
    ipv4_set_device_address(net_dev->fd, 0x0, 0x0);

    SYS_LOG_FUNCTION_EXIT(IPV4);

} /* ipv4_device_initialize */

/*
 * ipv4_compare_address
 * @address1:  Address needed to be matched.
 * @address2: Given address.
 * @match: Current match status.
 * @return: A true will be returned if addresses match exactly, partial will be
 *  returned if they match partially and false will be returned if address don't
 *  match at all.
 * This function will match two IPv4 addresses.
 */
uint8_t ipv4_compare_address(uint32_t address1, uint32_t address2, uint8_t match)
{
    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* If match is not already failed. */
    if (match != FALSE)
    {
        /* Compare the two IPv4 addresses. */

        /* If we don't have an unspecified address. */
        if (address1 != IPV4_ADDR_UNSPEC)
        {
            /* If the address exactly match. */
            if (address1 == address2)
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

    SYS_LOG_FUNCTION_EXIT(IPV4);

    /* Return match status to the caller. */
    return (match);

} /* ipv4_compare_address */

/*
 * ipv4_get_device_address
 * @fd: File descriptor associated with the networking device.
 * @address: IPv4 address associated with this device will be returned here.
 * @subnet: Subnet mask for this address will be returned here.
 * @return: A success status will be returned if IPv4 address was successfully
 *  returned, NET_INVALID_FD will be returned if the given file descriptor don't
 *  have an associated networking device.
 * This function will return IPv4 address associated with the networking device
 * as resolved from the file descriptor.
 */
int32_t ipv4_get_device_address(FD fd, uint32_t *address, uint32_t *subnet)
{
    NET_DEV *net_device = net_device_get_fd(fd);
    int32_t status = SUCCESS;

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    if (net_device != NULL)
    {
        /* Search for the required address. */
        if (route_get(&fd, IPV4_ADDR_BCAST, address, NULL, subnet) != SUCCESS)
        {
            /* If we need to return the address. */
            if (address != NULL)
            {
                /* Address is not yet assigned. */
                *address = IPV4_ADDR_UNSPEC;
            }

            /* If we need to return the subnet mask. */
            if (subnet != NULL)
            {
                /* Return the associated subnet mask. */
                *subnet = 0x0;
            }

            status = SUCCESS;
        }
    }
    else
    {
        /* We did not find a valid networking device for given file descriptor. */
        status = NET_INVALID_FD;
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(IPV4, status);

    /* Return status to the caller. */
    return (status);

} /* ipv4_get_device_address */

/*
 * ipv4_set_device_address
 * @fd: File descriptor associated with the networking device.
 * @address: IPv4 address needed to be updated.
 * @subnet: Subnet for this address.
 * @return: A success status will be returned if IPv4 address was successfully
 *  returned, NET_INVALID_FD will be returned if the given file descriptor don't
 *  have an associated networking device.
 * This function will update the IPv4 address associated with the networking
 * device as resolved from the file descriptor.
 */
int32_t ipv4_set_device_address(FD fd, uint32_t address, uint32_t subnet)
{
    NET_DEV *net_device = net_device_get_fd(fd);
    int32_t status = SUCCESS;

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    if (net_device != NULL)
    {
        /* If we are not clearing old address. */
        if (address != 0x0)
        {
            /* Add a link local route for this IP address. */
            route_add(fd, address, IPV4_GATWAY_LL, address, subnet, 0);
        }
        else
        {
            /* Remove all the associated routes for this device. */
            route_remove(fd, IPV4_ADDR_UNSPEC, IPV4_ADDR_UNSPEC);
        }
    }
    else
    {
        /* We did not find a valid networking device for given file descriptor. */
        status = NET_INVALID_FD;
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(IPV4, status);

    /* Return status to the caller. */
    return (status);

} /* ipv4_set_device_address */

/*
 * ipv4_get_source_device
 * @address: IPv4 address for which device is required.
 * @return: If not null a device entry associated with given address will be
 *  returned.
 * This function will return a device associated with the address.
 */
NET_DEV *ipv4_get_source_device(uint32_t address)
{
    FD fd = NULL;

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* Disable preemption. */
    scheduler_lock();

    /* Search for the required device. */
    route_get(&fd, IPV4_ADDR_BCAST, &address, NULL, NULL);

    /* Enable scheduling. */
    scheduler_unlock();

    SYS_LOG_FUNCTION_EXIT(IPV4);

    /* Resolve and return the required device. */
    return (net_device_get_fd(fd));

} /* ipv4_get_source_device */

/*
 * net_process_ipv4
 * @buffer: Received networking buffer needed to be processed.
 * @flags: Associated flags for this buffer.
 * @return: A success status will be returned if IPv4 packet was successfully
 *  processed.
 *  NET_BUFFER_CONSUMED will be returned if buffer was consumed and
 *  should not be freed by the caller.
 *  NET_INVALID_HDR will be returned if an invalid header was parsed.
 *  NET_NOT_SUPPORTED will be returned if an unsupported IPv4 header was parsed.
 *  NET_NO_ACTION will be returned if we don't want to process this packet and
 *  drop it silently.
 * This function will process an incoming IPv4 packet.
 */
int32_t net_process_ipv4(FS_BUFFER *buffer, uint32_t flags)
{
    int32_t status = SUCCESS;
    uint32_t ip_dst, ip_src, subnet, ip_iface = 0;
    uint8_t proto, ver_ihl;
    uint16_t flag_offset, ip_length;
#ifdef NET_ICMP
    uint8_t keep, icmp_rep;
#endif

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* We must have at least one byte to verify an IPv4 packet. */
    if (buffer->total_length >= 1)
    {
        /* Peek the version and IHL. */
        ASSERT(fs_buffer_pull_offset(buffer, &ver_ihl, 1, IPV4_HDR_VER_IHL_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

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
        if (net_csum_calculate(buffer, ver_ihl, 0) != 0)
        {
            /* Return an error. */
            status = NET_INVALID_CSUM;
        }
    }

    if (status == SUCCESS)
    {
        /* Pull the flag and offset data from the buffer. */
        ASSERT(fs_buffer_pull_offset(buffer, &flag_offset, 2, IPV4_HDR_FLAG_FRAG_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* Pick the IPv4 address to which this packet was addressed to. */
        ASSERT(fs_buffer_pull_offset(buffer, &ip_dst, 4, IPV4_HDR_DST_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* Get IPv4 address assigned to this device. */
        ip_iface = ip_dst;
        ASSERT(ipv4_get_device_address(buffer->fd, &ip_iface, &subnet) != SUCCESS);

        /* Pull the IPv4 length for this packet. */
        ASSERT(fs_buffer_pull_offset(buffer, &ip_length, 2, IPV4_HDR_LENGTH_OFFSET, (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)));

        /* Check if we need to remove buffer padding. */
        if (ip_length < buffer->total_length)
        {
            /* Pull padding from the buffer. */
            ASSERT(fs_buffer_pull(buffer, NULL, (buffer->total_length - ip_length), FS_BUFFER_TAIL) != SUCCESS);
        }

        /* If buffer don't have anticipated IP data. */
        else if (ip_length > buffer->total_length)
        {
            /* Return an error. */
            status = NET_INVALID_HDR;
        }
    }

    if (status == SUCCESS)
    {
        /* If this is a fragmented packet and this is intended for us. */
        if ((flag_offset & IPV4_HDR_FLAG_MF) || ((flag_offset & IPV4_HDR_FRAG_MASK) != 0))
        {
            /* Broadcast and multicast packets cannot be fragmented so if we are
             * not the destination just drop this packet. */
            if (((flags & ETH_FRAME_BCAST) == FALSE) && (ip_iface != IPV4_ADDR_UNSPEC) && (ip_dst == ip_iface))
            {
#ifdef IPV4_ENABLE_FRAG
                /* Try to process this fragment. */
                status = ipv4_frag_add(buffer, flag_offset);
#else
                /* We don't support fragmentation. */
                status = NET_NOT_SUPPORTED;
#endif /* IPV4_ENABLE_FRAG */
            }
            else
            {
                /* No action is required for this packet. */
                status = NET_NO_ACTION;
            }
        }
    }

    if (status == SUCCESS)
    {
        /* Peek the IPv4 protocol. */
        ASSERT(fs_buffer_pull_offset(buffer, &proto, 1, IPV4_HDR_PROTO_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

        /* Pick the IPv4 address from which this packet came. */
        ASSERT(fs_buffer_pull_offset(buffer, &ip_src, 4, IPV4_HDR_SRC_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* Try to resolve the protocol to which this packet is needed to be
         * forwarded. */
        switch(proto)
        {
#ifdef NET_ICMP
        /* If an ICMP packet is received. */
        case IP_PROTO_ICMP:

            /* Process ICMP packet. */
            status = net_process_icmp(buffer, ver_ihl, ip_iface, ip_src, ip_dst);

            break;
#endif /* NET_ICMP */

#ifdef NET_TCP
        /* If a TCP packet is received. */
        case IP_PROTO_TCP:

            /* Process a TCP packet. */
            status = net_process_tcp(buffer, ver_ihl, ip_iface, ip_src, ip_dst);

            break;
#endif /* NET_TCP */

#ifdef NET_UDP
        /* If a UDP packet is received. */
        case IP_PROTO_UDP:

            /* Process a UDP packet. */
            status = net_process_udp(buffer, ver_ihl, ip_iface, ip_src, ip_dst);

            break;
#endif /* NET_UDP */

        /* A valid protocol was not resolved. */
        default:

            /* If this packet intended for us. */
            if (ip_dst == ip_iface)
            {
                /* Unknown protocol was received. */
                status = NET_UNKNOWN_PROTO;
            }
            else
            {
                /* Destination address is unreachable. */
                status = NET_DST_UNREACHABLE;
            }

            break;
        }

#ifdef NET_ICMP
        /* If packet was not parsed correctly and it is not a broadcast frame. */
        if ((status != SUCCESS) && (status != NET_BUFFER_CONSUMED) && ((flags & ETH_FRAME_BCAST) == FALSE) && (ip_dst != IPV4_ADDR_BCAST_NET(ip_iface, subnet)) && (ip_dst != IPV4_ADDR_BCAST))
        {
            /* Resolve an ICMP message needed be sent. */
            switch (status)
            {
            case NET_UNKNOWN_PROTO:

                /* Protocol not resolved. */
                icmp_rep = ICMP_DST_PROTO;

                break;

            case NET_DST_UNREACHABLE:

                /* Cannot forward this packet, destination unreachable. */
                icmp_rep = ICMP_DST_HOST;

                break;

            case NET_DST_PRT_UNREACHABLE:

                /* Destination port is unreachable. */
                icmp_rep = ICMP_DST_PORT;

                break;

            default:

                /* No need to send an ICMP reply. */
                icmp_rep = ICMP_DST_NONE;

                break;
            }

            /* If we need to send an ICMP packet. */
            if (icmp_rep != ICMP_DST_NONE)
            {
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
                    ASSERT(fs_buffer_pull(buffer, NULL, (buffer->total_length - keep), FS_BUFFER_TAIL) != SUCCESS);
                }

                /* Generate an ICMP protocol unreachable message. */
                status = icmp_header_add(buffer, ICMP_DST_UNREACHABLE, icmp_rep, 0);

                if (status == SUCCESS)
                {
                    /* Add IPv4 packet on the packet. */
                    /* We will be sending a packet from our interface to the host it came from. */
                    status = ipv4_header_add(buffer, IP_PROTO_ICMP, ip_iface, ip_src, 0);
                }

                if (status == SUCCESS)
                {
                    /* Transmit an IPv4 packet. */
                    status = net_device_buffer_transmit(buffer, NET_PROTO_IPV4, 0);
                }
            }
        }
#endif /* NET_ICMP */
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(IPV4, (status == NET_BUFFER_CONSUMED) ? SUCCESS : status);

    /* Return status to the caller. */
    return (status);

} /* net_process_ipv4 */

/*
 * ipv4_header_add
 * @buffer: File system buffer on which IPv4 header is needed to be added.
 * @proto: Protocol filed value for IPv4 header.
 * @src_addr: Source address for this IPv4 packet.
 * @dst_addr: Destination address for this IPv4 packet.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @return: A success status will be returned if IPv4 header was successfully
 *  added.
 * This function will add an IPv4 header on the given buffer. If required buffer
 * will also be fragmented according to the MTU of the device on which this packet
 * will be sent.
 */
int32_t ipv4_header_add(FS_BUFFER *buffer, uint8_t proto, uint32_t src_addr, uint32_t dst_addr, uint8_t flags)
{
    int32_t status = SUCCESS;
    HDR_GEN_MACHINE hdr_machine;
    uint8_t ver_ihl, ihl, dscp = 0, ttl = 128;
    uint16_t flag_offset, csum, this_length, offset = 0;
    uint32_t total_length;
#ifdef IPV4_ENABLE_FRAG
    uint32_t max_payload_len;
#endif
    static uint16_t id = 0;
    HEADER headers[] =
    {
        {&ver_ihl,      1,  flags },                        /* Version and IHL. */
        {&dscp,         1,  flags },                        /* DSCP. */
        {&this_length,  2,  (FS_BUFFER_PACKED | flags) },   /* Total length. */
        {&id,           2,  (FS_BUFFER_PACKED | flags) },   /* Fragment ID. */
        {&flag_offset,  2,  (FS_BUFFER_PACKED | flags) },   /* Flags and fragment offset. */
        {&ttl,          1,  flags },                        /* Time to live. */
        {&proto,        1,  flags },                        /* Protocol. */
        {&csum,         2,  flags },                        /* Checksum. */
        {&src_addr,     4,  (FS_BUFFER_PACKED | flags) },   /* Source address. */
        {&dst_addr,     4,  (FS_BUFFER_PACKED | flags) },   /* Destination address. */
    };

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* Increment the ID for each packet we send. */
    id++;

    /* Calculate the IPv4 header length. */
    ihl = (ALLIGN_CEIL_N((IPV4_HDR_SIZE), 4) >> 2);

#ifdef IPV4_ENABLE_FRAG
    /* Calculate the maximum number of bytes that can be sent in a single
     * IPv4 packet. */
    /* The fragment offset is measured in units of 8 octets (64 bits). */
    max_payload_len = ALLIGN_FLOOR_N((net_device_get_mtu(buffer->fd) - (uint32_t)(ihl << 2)), 8);
#else
    /* Payload should never be greater than the number of bytes we can sent in
     * one datagram. */
    ASSERT((net_device_get_mtu(buffer->fd) - (uint32_t)(ihl << 2)) < buffer->total_length);
#endif

    /* Save the total number of bytes of payload we need to send. */
    total_length = buffer->total_length;

    /* Clear the next buffer pointer. */
    buffer->next = NULL;

    /* While we have some payload to send. */
    while (total_length > 0)
    {
        /* Initialize header values. */
        csum = 0;
        flag_offset = 0;

#ifdef IPV4_ENABLE_FRAG
        /* Check if we need to fragment this buffer. */
        if (total_length > max_payload_len)
        {
            /* Divide the given buffer into two buffers. */
            ASSERT(fs_buffer_divide(buffer, 0, max_payload_len) != SUCCESS);

            /* We will be sending more fragments after this. */
            flag_offset |= IPV4_HDR_FLAG_MF;
        }
#endif

        /* Initialize the total length field for this buffer. */
        this_length = (uint16_t)(buffer->total_length + (uint32_t)(ihl << 2));

        /* Decrement the number of bytes in payload we need to process. */
        total_length -= buffer->total_length;

        /* Offset should always be divisible by 8. */
        ASSERT((offset > 0) && (offset % 8));

        /* Set the offset for this fragment. */
        flag_offset |= (offset >> 3);

        /* Save the offset for next fragment. */
        offset = (uint16_t)(offset + buffer->total_length);

        /* Create the version and header length headers. */
        ver_ihl = (ihl | IPV4_HDR_VER);

        /* Initialize header generator machine. */
        header_gen_machine_init(&hdr_machine, &fs_buffer_hdr_push);

        /* Push the IPv4 header on the buffer. */
        status = header_generate(&hdr_machine, headers, sizeof(headers)/sizeof(HEADER), buffer);

        if (status == SUCCESS)
        {
            /* Compute and update the value of checksum field. */
            csum = net_csum_calculate(buffer, (ihl << 2), 0);
            status = fs_buffer_push_offset(buffer, &csum, 2, IPV4_HDR_CSUM_OFFSET, (FS_BUFFER_HEAD | FS_BUFFER_UPDATE));
        }

        if (status == SUCCESS)
        {
            /* Pick the remaining data of this buffer. */
            buffer = buffer->next;
        }
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(IPV4, status);

    /* Return status to the caller. */
    return (status);

} /* ipv4_header_add */

#ifdef IPV4_ENABLE_FRAG

/*
 * ipv4_fragment_set_data
 * @fd: File descriptor associated with a networking device for which fragment
 * data is needed to be set.
 * @fragments: IPv4 fragment list.
 * @num: Number of IPv4 fragments.
 * This function will initialize IPv4 fragment data for an networking device.
 */
void ipv4_fragment_set_data(FD fd, IPV4_FRAGMENT *fragments, uint32_t num)
{
    NET_DEV *net_device = net_device_get_fd(fd);

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* Initialize the IPv4 fragment data. */
    net_device->ipv4.fargment.list = fragments;
    net_device->ipv4.fargment.num = num;

    /* Initialize the IPv4 fragment condition. */
    net_device->ipv4.fargment.suspend.timeout_enabled = FALSE;
    net_device->ipv4.fargment.suspend.priority = NET_IPV4_PRIORITY;

    /* Add condition for this fragment in networking stack. */
    net_condition_add(&net_device->ipv4.fargment.condition, &net_device->ipv4.fargment.suspend, &ipv4_fragment_expired, net_device);

    SYS_LOG_FUNCTION_EXIT(IPV4);

} /* ipv4_fragment_set_data */

/*
 * ipv4_fragment_update_timer
 * @net_device: Networking device for which IPv4 fragment timer is needed to be
 *  updated.
 * This function will update IPv4 fragment timer for given networking device.
 */
static void ipv4_fragment_update_timer(NET_DEV *net_device)
{
    uint32_t next_timeout;
    uint32_t n;
    uint8_t timeout_enabled = FALSE;

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* Go though all the fragments in this device. */
    for (n = 0; n < net_device->ipv4.fargment.num; n++)
    {
        /* If this fragment is being used. */
        if (net_device->ipv4.fargment.list[n].flags & IPV4_FRAG_IN_USE)
        {
            /* If this fragment will expire before the last saved expire time. */
            if ((timeout_enabled == FALSE) || (INT32CMP(next_timeout, net_device->ipv4.fargment.list[n].timeout) > 0))
            {
                /* Use this fragment's timeout. */
                next_timeout = net_device->ipv4.fargment.list[n].timeout;

                /* timeout is now enabled. */
                timeout_enabled = TRUE;
            }
        }
    }

    /* If we need to enable timeout for fragmentation. */
    if (timeout_enabled == TRUE)
    {
        /* Save the timeout at which we will need to expire next fragment. */
        net_device->ipv4.fargment.suspend.timeout = next_timeout;
        net_device->ipv4.fargment.suspend.timeout_enabled = TRUE;
    }
    else
    {
        /* Disable the fragmentation timer. */
        net_device->ipv4.fargment.suspend.timeout_enabled = FALSE;
    }

    SYS_LOG_FUNCTION_EXIT(IPV4);

} /* ipv4_fragment_update_timer */

/*
 * ipv4_fragment_expired
 * @data: Networking device for which IPv4 fragments are needed to be
 * processed.
 * @status: Resumption status.
 * This function will called when we timeout receiving data for a fragment.
 */
static void ipv4_fragment_expired(void *data, int32_t status)
{
    NET_DEV *net_device = (NET_DEV *)data;
    uint32_t n, clock = current_system_tick();
    FD buffer_fd;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(status);

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* Go though all the fragments in this device. */
    for (n = 0; n < net_device->ipv4.fargment.num; n++)
    {
        /* If this fragment is now expired. */
        if ((net_device->ipv4.fargment.list[n].flags & IPV4_FRAG_IN_USE) && (INT32CMP(clock, net_device->ipv4.fargment.list[n].timeout) >= 0))
        {
            /* If we do have at least one buffer on this fragment. */
            if (net_device->ipv4.fargment.list[n].buffer_list.head)
            {
                /* Save the file descriptor on which data was received. */
                buffer_fd = net_device->ipv4.fargment.list[n].buffer_list.head->fd;

                /* Obtain lock for the file descriptor on which this buffer was
                 * received. */
                ASSERT(fd_get_lock(buffer_fd) != SUCCESS);

                /* Free this buffer list. */
                fs_buffer_add_buffer_list(net_device->ipv4.fargment.list[n].buffer_list.head, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

                /* Release semaphore for the buffer. */
                fd_release_lock(buffer_fd);
            }

            /* Clear the fragment data. */
            memset(&net_device->ipv4.fargment.list[n], 0, sizeof(IPV4_FRAGMENT));
        }
    }

    /* Update fragment timer. */
    ipv4_fragment_update_timer(net_device);

    SYS_LOG_FUNCTION_EXIT(IPV4);

} /* ipv4_fragment_expired */

/*
 * ipv4_frag_sort
 * @node: An existing node in the fragment list.
 * @new_node: New node we need to add in the list.
 * @return: Return true if this fragment is needed to be added before this
 *  fragment otherwise false will be returned.
 * This function will see if new fragment is needed to be added before the given
 * fragment.
 */
static uint8_t ipv4_frag_sort(void *node, void *new_node)
{
    FS_BUFFER *this_buffer = (FS_BUFFER *)node, *buffer = (FS_BUFFER *)new_node;
    uint16_t this_flag_offset, flag_offset;
    uint8_t insert = FALSE;

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* Pull the flag and offset field for these buffers. */
    ASSERT(fs_buffer_pull_offset(this_buffer, &this_flag_offset, 2, IPV4_HDR_FLAG_FRAG_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);
    ASSERT(fs_buffer_pull_offset(buffer, &flag_offset, 2, IPV4_HDR_FLAG_FRAG_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

    /* If existing node's offset is greater then the one we have received. */
    if ((this_flag_offset & IPV4_HDR_FRAG_MASK) >= (flag_offset & IPV4_HDR_FRAG_MASK))
    {
        /* Insert this fragment here. */
        insert = TRUE;
    }

    SYS_LOG_FUNCTION_EXIT(IPV4);

    /* Return if we need to insert this fragment here. */
    return (insert);

} /* ipv4_frag_sort */

/*
 * ipv4_frag_add
 * @buffer: IPv4 fragment received.
 * @flag_offset: Flag and fragment offset as parsed from the buffer.
 * @return: A success status will be returned if all the fragments for this
 *  packet are now received and we can now process this packet. NET_NO_ACTION
 *  will be returned if we don't have a complete fragment and we will need for
 *  more data to process this packet. FS_BUFFER_NO_SPACE will be returned if we
 *  don't have a free fragment or buffer list to process this packet.
 * This function will add a new fragment for the file descriptor.
 */
static int32_t ipv4_frag_add(FS_BUFFER *buffer, uint16_t flag_offset)
{
    NET_DEV *net_device = net_device_get_fd(buffer->fd);
    FS_BUFFER *tmp_buffer;
    IPV4_FRAGMENT *fragment = NULL;
    uint32_t sa, n;
    int32_t status = FS_BUFFER_NO_SPACE;
    uint16_t id;

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* Should never happen. */
    ASSERT(net_device == NULL);

    /* Pull the ID of this fragment. */
    ASSERT(fs_buffer_pull_offset(buffer, &id, 2, IPV4_HDR_ID_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

    /* Pull the source address to which we will be sending the reply. */
    ASSERT(fs_buffer_pull_offset(buffer, &sa, 4, IPV4_HDR_SRC_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

    /* Search all the fragments for a free one. */
    for (n = 0; n < net_device->ipv4.fargment.num; n++)
    {
        /* If this fragment list is free. */
        if ((fragment == NULL) &&
            ((net_device->ipv4.fargment.list[n].flags & IPV4_FRAG_IN_USE) == 0))
        {
            /* Save the fragment as it is free. */
            fragment = &net_device->ipv4.fargment.list[n];
        }

        /* If we already have a fragment list for this fragment */
        else if ((net_device->ipv4.fargment.list[n].sa == sa) && (net_device->ipv4.fargment.list[n].id == id))
        {
            /* Use this fragment to reassemble the packet. */
            fragment = &net_device->ipv4.fargment.list[n];

            /* Break out of this loop. */
            break;
        }
    }

    /* If do have a fragment list to process this fragment. */
    if (fragment != NULL)
    {
        /* If threshold buffers are now being consumed. */
        if (fs_buffer_threshold_locked(buffer->fd))
        {
            /* We will drop this fragment and any other fragments that are
             * accumulated as threshold is activated and we would never able to
             * complete the enqueued fragments. */
            /* If we could identify the fragment size before receiving the last
             * fragment this could have been avoided. */
            for (n = 0; n < net_device->ipv4.fargment.num; n++)
            {
                /* If this fragment is in use. */
                if (net_device->ipv4.fargment.list[n].flags & IPV4_FRAG_IN_USE)
                {
                    /* Free any fragments we have already on this fragment list. */
                    fs_buffer_add_buffer_list(net_device->ipv4.fargment.list[n].buffer_list.head, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

                    /* Clear the buffer list for this fragment. */
                    net_device->ipv4.fargment.list[n].buffer_list.head = net_device->ipv4.fargment.list[n].buffer_list.tail = NULL;

                    /* Set flag to drop any more packets received for this fragment. */
                    net_device->ipv4.fargment.list[n].flags |= IPV4_FRAG_DROP;

                    /* Expire this fragment after drop timeout. */
                    fragment->timeout = (MS_TO_TICK(IPV4_FRAG_DROP_TIMEOUT) + current_system_tick());
                }
            }

            /* Update fragment timer. */
            ipv4_fragment_update_timer(net_device);
        }

        /* If we are not dropping this fragment. */
        else if ((fragment->flags & IPV4_FRAG_DROP) == 0)
        {
            /* Try to allocate a temporary buffer list keeping threshold buffers. */
            tmp_buffer = fs_buffer_get(buffer->fd, FS_BUFFER_LIST, FS_BUFFER_TH);

            /* If a temporary buffer list was successfully allocated. */
            if (tmp_buffer != NULL)
            {
                /* If this is a new fragment. */
                if ((fragment->flags & IPV4_FRAG_IN_USE) == 0)
                {
                    /* Initialize this fragment. */
                    fragment->flags |= IPV4_FRAG_IN_USE;
                    fragment->id = id;
                    fragment->sa = sa;

                    /* Save the timeout at which we will need to expire this fragment. */
                    fragment->timeout = (MS_TO_TICK(IPV4_FRAG_TIMEOUT) + current_system_tick());

                    /* Update fragment timer. */
                    ipv4_fragment_update_timer(net_device);
                }

                /* Move data from the original buffer to the temporary buffer. */
                fs_buffer_move(tmp_buffer, buffer);

                /* Push this fragment on the fragment list. */
                sll_insert(&fragment->buffer_list, tmp_buffer, &ipv4_frag_sort, OFFSETOF(FS_BUFFER, next));

                /* If this is last fragment. */
                if ((flag_offset & IPV4_HDR_FRAG_MASK) == 0)
                {
                    /* Set the first fragment received flag. */
                    fragment->flags |= IPV4_FRAG_HAVE_FIRST;
                }

                /* If we have received the last fragment. */
                if (((flag_offset & IPV4_HDR_FLAG_MF) == 0) || (fragment->flags & IPV4_FRAG_LAST_RCVD))
                {
                    /* Set the last fragment received flag. */
                    fragment->flags |= IPV4_FRAG_LAST_RCVD;
                }

                /* Merge the received fragments on the go. */
                status = ipv4_frag_merge(fragment, buffer);
            }
        }
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(IPV4, status);

    /* Return status to the caller. */
    return (status);

} /* ipv4_frag_add */

/*
 * ipv4_frag_merge
 * @fragment: Fragment needed to be merged.
 * @buffer: If fragment is complete this buffer will be be updated to contain
 * the reassembled packet.
 * @return: A success status will be returned if a packet was successfully
 *  constructed out of fragments and can now be processed.
 *  NET_NO_ACTION will be returned if more fragments are required to
 *  construct the complete IPv4 packet.
 * This function will merge all the fragments in to one buffer that can be
 * further processed.
 */
static int32_t ipv4_frag_merge(IPV4_FRAGMENT *fragment, FS_BUFFER *buffer)
{
    FS_BUFFER *last_buffer, *next_buffer, *tmp_buffer;
    int32_t status = SUCCESS;
    uint16_t flag_offset, next_offset = 0;
    uint8_t ver_ihl;

    SYS_LOG_FUNCTION_ENTRY(IPV4);

    /* Pick the first and next buffer. */
    last_buffer = fragment->buffer_list.head;
    next_buffer = last_buffer->next;

    /* Pull the version and IHL fields for the first buffer. */
    ASSERT(fs_buffer_pull_offset(last_buffer, &ver_ihl, 1, IPV4_HDR_VER_IHL_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

    /* Pull the flag and offset field for this buffer. */
    ASSERT(fs_buffer_pull_offset(last_buffer, &flag_offset, 2, IPV4_HDR_FLAG_FRAG_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

    /* Calculate the anticipated offset for next fragment. */
    next_offset = (uint16_t)((((int32_t)last_buffer->total_length - ((ver_ihl & IPV4_HDR_IHL_MASK) << 2)) + ((flag_offset & IPV4_HDR_FRAG_MASK) << 3)) >> 3);

    /* While we have a buffer in the fragment list. */
    while (next_buffer)
    {
        /* Pull the version and IHL fields for this buffer. */
        ASSERT(fs_buffer_pull_offset(next_buffer, &ver_ihl, 1, IPV4_HDR_VER_IHL_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

        /* Pull the flag and offset field for this buffer. */
        ASSERT(fs_buffer_pull_offset(next_buffer, &flag_offset, 2, IPV4_HDR_FLAG_FRAG_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* If there is no hole between last fragment and this fragment. */
        if ((flag_offset & IPV4_HDR_FRAG_MASK) == next_offset)
        {
            /* Pull the IPv4 header from this buffer. */
            ASSERT(fs_buffer_pull(next_buffer, NULL, (uint32_t)((ver_ihl & IPV4_HDR_IHL_MASK) << 2), 0) != SUCCESS);

            /* Move data from this buffer in the return buffer. */
            fs_buffer_move_data(last_buffer, next_buffer, 0);

            /* Save pointer for this buffer. */
            tmp_buffer = next_buffer;

            /* Pick the next buffer from the list. */
            next_buffer = next_buffer->next;

            /* Remove this buffer from the buffer list. */
            ASSERT(sll_remove(&fragment->buffer_list, tmp_buffer, OFFSETOF(FS_BUFFER, next)) != tmp_buffer);

            /* Add this buffer back to the buffer list. */
            fs_buffer_add(tmp_buffer->fd, tmp_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }

        else
        {
            /* There is a hole in the fragment list. */
            status = NET_NO_ACTION;

            /* Calculate the anticipated offset for next fragment. */
            next_offset = (uint16_t)((((int32_t)last_buffer->total_length - ((ver_ihl & IPV4_HDR_IHL_MASK) << 2)) + ((flag_offset & IPV4_HDR_FRAG_MASK) << 3)) >> 3);

            /* We will use this buffer to merge any next fragments. */
            last_buffer = next_buffer;

            /* Pick the next buffer from the list. */
            next_buffer = next_buffer->next;
        }
    }

    /* If no holes were found in the fragment list. */
    if (status == SUCCESS)
    {
        /* If first and last fragment has been received. */
        if ((fragment->flags & IPV4_FRAG_HAVE_FIRST) && (fragment->flags & IPV4_FRAG_LAST_RCVD))
        {
            /* Move data from the fragment head to the provided buffer. */
            fs_buffer_move(buffer, last_buffer);

            /* Free the fragment head. */
            fs_buffer_add(last_buffer->fd, last_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

            /* Clear the fragment data. */
            memset(fragment, 0, sizeof(IPV4_FRAGMENT));
        }
        else
        {
            /* We still need more data to process this packet. */
            status = NET_NO_ACTION;
        }
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(IPV4, status);

    /* Return status to the caller. */
    return (status);

} /* ipv4_frag_merge */

#endif /* IPV4_ENABLE_FRAG */

#endif /* NET_IPV4 */
#endif /* CONFIG_NET */
