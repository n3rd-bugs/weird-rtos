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
#ifdef NET_UDP
#include <net_udp.h>
#endif
#include <net_csum.h>
#include <sll.h>
#include <string.h>

/* Internal function prototypes. */
#ifdef IPV4_ENABLE_FRAG
static void ipv4_fragment_initailize(IPV4_FRAGMENT *);
static void ipv4_fragment_reset(IPV4_FRAGMENT *);
static void ipv4_fragment_expired(void *);
static uint8_t ipv4_frag_sort(void *, void *);
static int32_t ipv4_frag_add(FS_BUFFER **, uint16_t);
static int32_t ipv4_frag_merge(IPV4_FRAGMENT *, FS_BUFFER **);
#endif

/*
 * ipv4_device_initialize
 * @net_dev: Networking device for which IPv4 data is needed to be initialized.
 * This function will initialize IPv4 data for a new networking device.
 */
void ipv4_device_initialize(NET_DEV *net_dev)
{
    uint32_t n;

    /* Clear the IPv4 address assigned to this device. */
    net_dev->ipv4_address = 0;

#ifdef IPV4_ENABLE_FRAG
    /* Initialize all IPv4 fragments. */
    for (n = 0; n < IPV4_NUM_FRAGS; n++)
    {
        /* Initialize IPv4 fragment data. */
        ipv4_fragment_initailize(&net_dev->ipv4_fragments[n]);
    }
#endif /* IPV4_ENABLE_FRAG */

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

    /* Return match status to the caller. */
    return (match);

} /* ipv4_compare_address */

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
        /* Return the IPv4 address assigned to this device. */
        *address = net_device->ipv4_address;
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
        /* Update the IPv4 address assigned to this device. */
        net_device->ipv4_address = address;
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
 * ipv4_sreach_device
 * @node: An existing device.
 * @param: IPv4 source address for which source device is required.
 * @return: True will be returned if this is the required device, otherwise
 *  false will be returned.
 * This is a search function to find a source device for a given IPv4 source
 * address.
 */
uint8_t ipv4_sreach_device(void *node, void *param)
{
    NET_DEV *net_device = (NET_DEV *)node;
    uint32_t address = (uint32_t)param;
    uint8_t match = FALSE;

    /* If this is the required device. */
    if (net_device->ipv4_address == address)
    {
        /* Got an match. */
        match = TRUE;
    }

    /* Return if this is the required device. */
    return (match);

} /* ipv4_sreach_device */

/*
 * ipv4_get_source_device
 * @address: IPv4 address for which device is required.
 * @return: If not null a device entry associated with given address will be
 *  returned.
 * This function will return a device associated with the address.
 */
NET_DEV *ipv4_get_source_device(uint32_t address)
{
    NET_DEV *ret_device;
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Search for the required device. */
    ret_device = sll_search(&net_dev_data.devices, NULL, &ipv4_sreach_device, (void *)address, OFFSETOF(NET_DEV, next));

    /* Restore the IRQ interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Return the resolved device for the given IP address. */
    return (ret_device);

} /* ipv4_get_source_device */

/*
 * net_process_ipv4
 * @net_buffer: Pointer to file system buffer needed to be processed, this will
 *  be updated if interchanged during processing.
 * @return: A success status will be returned if IPv4 packet was successfully
 *  processed.
 *  NET_BUFFER_CONSUMED will be returned if buffer was consumed and
 *  should not be freed by the caller.
 *  NET_INVALID_HDR will be returned if an invalid header was parsed.
 *  NET_NOT_SUPPORTED will be returned if an unsupported IPv4 header was parsed.
 * This function will process an incoming IPv4 packet.
 */
int32_t net_process_ipv4(FS_BUFFER **net_buffer)
{
    FS_BUFFER *buffer = (*net_buffer);
    int32_t status = SUCCESS;
    uint32_t ip_iface = 0, ip_dst, ip_src;
    uint8_t proto, keep, ver_ihl, icmp_rep;
    uint16_t flag_offset;

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
        if (net_csum_calculate(buffer, ver_ihl, 0) != 0)
        {
            /* Return an error. */
            status = NET_INVALID_HDR;
        }
    }

    if (status == SUCCESS)
    {
        /* Verify that this is not a fragmented packet, as we don't support
         * it. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &flag_offset, 2, IPV4_HDR_FLAG_FRAG_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* If this packet will be fragmented. */
        if ((flag_offset & IPV4_HDR_FALG_MF) || ((flag_offset & IPV4_HDR_FRAG_MASK) != 0))
        {
#ifdef IPV4_ENABLE_FRAG
            /* Try to process this fragment. */
            status = ipv4_frag_add(&buffer, flag_offset);
#else
            /* We don't support fragmentation. */
            status = NET_NOT_SUPPORTED;
#endif /* IPV4_ENABLE_FRAG */
        }
    }

    if (status == SUCCESS)
    {
        /* Peek the IPv4 protocol and see if we do support it. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &proto, 1, IPV4_HDR_PROTO_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

        /* Pick the IPv4 address from which this packet came. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &ip_src, 4, IPV4_HDR_SRC_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* Pick the IPv4 address to which this packet was addressed to. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &ip_dst, 4, IPV4_HDR_DST_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* Release semaphore for the buffer file descriptor. */
        fd_release_lock(buffer->fd);

        /* Get IPv4 address assigned to this device. */
        OS_ASSERT(ipv4_get_device_address(buffer->fd, &ip_iface) != SUCCESS);

        /* Obtain lock for buffer file descriptor. */
        OS_ASSERT(fd_get_lock(buffer->fd) != SUCCESS);

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

#ifdef NET_UDP
        /* If a UDP packet is received. */
        case IP_PROTO_UDP:

            /* Process a UDP packet. */
            status = net_process_udp(buffer, ver_ihl, ip_iface, ip_src, ip_dst);

            break;
#endif /* NET_ICMP */

        /* A valid protocol was not resolved. */
        default:

            /* If this packet for intended for us. */
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
        /* If packet was not parsed correctly. */
        if ((status != SUCCESS) && (status != NET_BUFFER_CONSUMED))
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
                    OS_ASSERT(fs_buffer_pull(buffer, NULL, (buffer->total_length - keep), FS_BUFFER_TAIL) != SUCCESS);
                }

                /* Generate an ICMP protocol unreachable message. */
                status = icmp_header_add(buffer, ICMP_DST_UNREACHABLE, icmp_rep, 0);

                if (status == SUCCESS)
                {
                    /* Add IPv4 packet on the packet. */
                    /* We will be sending a packet from our interface to the host it came from. */
                    status = ipv4_header_add(buffer, IP_PROTO_ICMP, ip_iface, ip_src);
                }

                /* Transmit an IPv4 packet. */
                if (net_device_buffer_transmit(buffer, NET_PROTO_IPV4) == SUCCESS)
                {
                    /* We have transmitted the same buffer. */
                    status = NET_BUFFER_CONSUMED;
                }
            }
#endif /* NET_ICMP */
        }
    }

    /* Update the buffer pointer. */
    *net_buffer = buffer;

    /* Return status to the caller. */
    return (status);

} /* net_process_ipv4 */

/*
 * ipv4_header_add
 * @buffer: File system buffer on which IPv4 header is needed to be added.
 * @proto: Protocol filed value for IPv4 header.
 * @src_addr: Source address for this IPv4 packet.
 * @dst_addr: Destination address for this IPv4 packet.
 * @return: A success status will be returned if IPv4 header was successfully
 *  added.
 * This function will add an IPv4 header on the given buffer. If required buffer
 * will also be fragmented according to the MTU of the device on which this packet
 * will be sent.
 */
int32_t ipv4_header_add(FS_BUFFER *buffer, uint8_t proto, uint32_t src_addr, uint32_t dst_addr)
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
        {&ver_ihl,      1,  0 },                    /* Version and IHL. */
        {&dscp,         1,  0 },                    /* DSCP. */
        {&this_length,  2,  FS_BUFFER_PACKED },     /* Total length. */
        {&id,           2,  FS_BUFFER_PACKED },     /* Fragment ID. */
        {&flag_offset,  2,  FS_BUFFER_PACKED },     /* Flags and fragment offset. */
        {&ttl,          1,  0 },                    /* Time to live. */
        {&proto,        1,  0 },                    /* Protocol. */
        {&csum,         2,  0 },                    /* Checksum. */
        {&src_addr,     4,  FS_BUFFER_PACKED },     /* Source address. */
        {&dst_addr,     4,  FS_BUFFER_PACKED },     /* Destination address. */
    };

    /* Increment the ID for each packet we send. */
    id++;

    /* Calculate the IPv4 header length. */
    ihl = (ALLIGN_CEIL_N((IPV4_HDR_SIZE), 4) >> 2);

    /* Release semaphore for the buffer file descriptor. */
    fd_release_lock(buffer->fd);

#ifdef IPV4_ENABLE_FRAG
    /* Calculate the maximum number of bytes that can be sent in a single
     * IPv4 packet. */
    /* The fragment offset is measured in units of 8 octets (64 bits). */
    max_payload_len = ALLIGN_FLOOR_N((net_device_get_mtu(buffer->fd) - (uint32_t)(ihl << 2)), 8);
#else
    /* Payload should never be greater than the number of bytes we can sent in
     * one datagram. */
    OS_ASSERT((net_device_get_mtu(buffer->fd) - (uint32_t)(ihl << 2)) < buffer->total_length);
#endif

    /* Obtain lock for buffer file descriptor. */
    OS_ASSERT(fd_get_lock(buffer->fd) != SUCCESS);

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
            OS_ASSERT(fs_buffer_divide(buffer, max_payload_len) != SUCCESS);

            /* We will be sending more fragments after this. */
            flag_offset |= IPV4_HDR_FALG_MF;
        }
#endif

        /* Initialize the total length field for this buffer. */
        this_length = (uint16_t)(buffer->total_length + (uint32_t)(ihl << 2));

        /* Decrement the number of bytes in payload we need to process. */
        total_length -= buffer->total_length;

        /* Offset should always be divisible by 8. */
        OS_ASSERT((offset > 0) && (offset % 8));

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

    /* Return status to the caller. */
    return (status);

} /* ipv4_header_add */

#ifdef IPV4_ENABLE_FRAG

/*
 * ipv4_fragment_initailize
 * @fragment: IPv4 fragment needed to be initialized.
 * This function will initialize an IPv4 fragment.
 */
static void ipv4_fragment_initailize(IPV4_FRAGMENT *fragment)
{
    /* Fragment data is already cleared. */

    /* Initialize fragment condition as a timer condition. */
    fragment->suspend.flags = CONDITION_TIMER;
    fragment->suspend.timeout = IPV4_FRAG_TIMEOUT;

} /* ipv4_fragment_initailize */

/*
 * ipv4_fragment_reset
 * @fragment: IPv4 fragment needed to be reset.
 * This function will reset the fragment data.
 */
static void ipv4_fragment_reset(IPV4_FRAGMENT *fragment)
{
    /* Clear the fragment data. */
    memset(fragment, 0, sizeof(IPV4_FRAGMENT));

    /* Initialize the fragment structure. */
    ipv4_fragment_initailize(fragment);

} /* ipv4_fragment_reset */

/*
 * ipv4_fragment_expired
 * @data: IPv4 fragment for which time expired.
 * This function will called when we timeout receiving data for a fragment.
 */
static void ipv4_fragment_expired(void *data)
{
    IPV4_FRAGMENT *fragment = (IPV4_FRAGMENT *)data;
    FS_BUFFER *buffer, *tmp_buffer;
    FD buffer_fd;

    /* Pick the head buffer. */
    buffer = fragment->buffer_list.head;

    OS_ASSERT(buffer == NULL);

    /* Save the file descriptor on which data was received. */
    buffer_fd = buffer->fd;

    /* Obtain lock for the file descriptor on which this semaphore was
     * received. */
    OS_ASSERT(fd_get_lock(buffer_fd) != SUCCESS);

    /* Free all the buffers in this fragment. */
    while (buffer != NULL)
    {
        /* Save the next buffer. */
        tmp_buffer = buffer->next;

        /* Free this buffer. */
        fs_buffer_add(buffer->fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

        /* Pick the next buffer. */
        buffer = tmp_buffer;
    }

    /* Release semaphore for the buffer. */
    fd_release_lock(buffer_fd);

    /* Remove condition for this fragment. */
    net_condition_remove(&fragment->condition);

    /* Reset the fragment. */
    ipv4_fragment_reset(fragment);

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

    /* Pull the flag and offset field for these buffers. */
    OS_ASSERT(fs_buffer_pull_offset(this_buffer, &this_flag_offset, 2, IPV4_HDR_FLAG_FRAG_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);
    OS_ASSERT(fs_buffer_pull_offset(buffer, &flag_offset, 2, IPV4_HDR_FLAG_FRAG_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

    /* If existing node's offset is greater then the one we have received. */
    if ((this_flag_offset & IPV4_HDR_FRAG_MASK) >= (flag_offset & IPV4_HDR_FRAG_MASK))
    {
        /* Insert this fragment here. */
        insert = TRUE;
    }

    /* Return if we need to insert this fragment here. */
    return (insert);

} /* ipv4_frag_sort */

/*
 * ipv4_frag_add
 * @buffer: IPv4 fragment received, if a fragment is completed a pointer to
 *  complete packet will be returned here, otherwise will remain unchanged.
 * @flag_offset: Flag and fragment offset as parsed from the buffer.
 * @return: A success status will be returned if all the fragments for this
 *  packet are now received and we can now process this packet.
 *  NET_BUFFER_CONSUMED will be returned if we don't have a complete fragment
 *  and we will need for more data to process this packet.
 * This function will add a new fragment for the file descriptor.
 */
static int32_t ipv4_frag_add(FS_BUFFER **buffer, uint16_t flag_offset)
{
    NET_DEV *net_device = net_device_get_fd((*buffer)->fd);
    IPV4_FRAGMENT *fragment = NULL;
    uint32_t sa;
    int32_t status = NET_BUFFER_CONSUMED, n;
    uint16_t id;

    /* Should never happen. */
    OS_ASSERT(net_device == NULL);

    /* Pull the ID of this fragment. */
    OS_ASSERT(fs_buffer_pull_offset(*buffer, &id, 2, IPV4_HDR_ID_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

    /* Pull the source address to which we will be sending the reply. */
    OS_ASSERT(fs_buffer_pull_offset(*buffer, &sa, 4, IPV4_HDR_SRC_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

    /* Search all the fragments for a free one. */
    for (n = 0; n < IPV4_NUM_FRAGS; n++)
    {
        /* If this fragment list is free. */
        if ((fragment == NULL) &&
            ((net_device->ipv4_fragments[n].flags & IPV4_FRAG_IN_USE) == 0))
        {
            /* Save the fragment as it is free. */
            fragment = &net_device->ipv4_fragments[n];
        }

        /* If we already have a fragment list for this fragment */
        else if ((net_device->ipv4_fragments[n].sa == sa) && (net_device->ipv4_fragments[n].id == id))
        {
            /* Use this fragment to reassemble the packet. */
            fragment = &net_device->ipv4_fragments[n];

            /* Break out of this loop. */
            break;
        }
    }

    /* If do have a fragment list to process this fragment. */
    if (fragment != NULL)
    {
        /* If this is a new fragment. */
        if ((fragment->flags & IPV4_FRAG_IN_USE) == 0)
        {
            /* Initialize this fragment. */
            fragment->flags |= IPV4_FRAG_IN_USE;
            fragment->id = id;
            fragment->sa = sa;

            /* Add condition for this fragment in networking stack. */
            net_condition_add(&fragment->condition, &fragment->suspend, &ipv4_fragment_expired, fragment);
        }

        /* Push this fragment on the fragment list. */
        sll_insert(&fragment->buffer_list, *buffer, &ipv4_frag_sort, OFFSETOF(FS_BUFFER, next));

        /* If this is last fragment. */
        if ((flag_offset & IPV4_HDR_FRAG_MASK) == 0)
        {
            /* Set the first fragment received flag. */
            fragment->flags |= IPV4_FRAG_HAVE_FIRST;
        }

        /* If we have received the last fragment. */
        if (((flag_offset & IPV4_HDR_FALG_MF) == 0) || (fragment->flags & IPV4_FRAG_LAST_RCVD))
        {
            /* Set the last fragment received flag. */
            fragment->flags |= IPV4_FRAG_LAST_RCVD;
        }

        /* Merge the received fragments on the go. */
        status = ipv4_frag_merge(fragment, buffer);
    }

    /* Return status to the caller. */
    return (status);

} /* ipv4_frag_add */

/*
 * ipv4_frag_merge
 * @fragment: Fragment needed to be merged.
 * @buffer: If fragment is complete a new buffer pointer will be returned here
 *  which can be used to process incoming data.
 * @return: A success status will be returned if a packet was successfully
 *  constructed out of fragments and can now be processed.
 *  NET_BUFFER_CONSUMED will be returned if more fragments are required to
 *  construct the complete IPv4 packet.
 * This function will merge all the fragments in to one buffer that can be
 * further processed.
 */
static int32_t ipv4_frag_merge(IPV4_FRAGMENT *fragment, FS_BUFFER **buffer)
{
    FS_BUFFER *last_buffer, *next_buffer, *tmp_buffer;
    int32_t status = SUCCESS;
    uint16_t flag_offset, next_offset = 0;
    uint8_t ver_ihl;

    /* Pick the first and next buffer. */
    last_buffer = fragment->buffer_list.head;
    next_buffer = last_buffer->next;

    /* Pull the version and IHL fields for the first buffer. */
    OS_ASSERT(fs_buffer_pull_offset(last_buffer, &ver_ihl, 1, IPV4_HDR_VER_IHL_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

    /* Pull the flag and offset field for this buffer. */
    OS_ASSERT(fs_buffer_pull_offset(last_buffer, &flag_offset, 2, IPV4_HDR_FLAG_FRAG_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

    /* Calculate the anticipated offset for next fragment. */
    next_offset = (uint16_t)((((int32_t)last_buffer->total_length - ((ver_ihl & IPV4_HDR_IHL_MASK) << 2)) + ((flag_offset & IPV4_HDR_FRAG_MASK) << 3)) >> 3);

    /* While we have a buffer in the fragment list. */
    while (next_buffer)
    {
        /* Pull the version and IHL fields for this buffer. */
        OS_ASSERT(fs_buffer_pull_offset(next_buffer, &ver_ihl, 1, IPV4_HDR_VER_IHL_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

        /* Pull the flag and offset field for this buffer. */
        OS_ASSERT(fs_buffer_pull_offset(next_buffer, &flag_offset, 2, IPV4_HDR_FLAG_FRAG_OFFSET, (FS_BUFFER_INPLACE | FS_BUFFER_PACKED)) != SUCCESS);

        /* If there is no hole between last fragment and this fragment. */
        if ((flag_offset & IPV4_HDR_FRAG_MASK) == next_offset)
        {
            /* Pull the IPv4 header from this buffer. */
            OS_ASSERT(fs_buffer_pull(next_buffer, NULL, (uint32_t)((ver_ihl & IPV4_HDR_IHL_MASK) << 2), 0) != SUCCESS);

            /* Merge this buffer in the return buffer. */
            last_buffer->list.tail->next = next_buffer->list.head;
            last_buffer->list.tail = next_buffer->list.tail;
            last_buffer->total_length += next_buffer->total_length;

            /* Clear the list for this buffer. */
            next_buffer->list.head = next_buffer->list.tail = NULL;
            tmp_buffer = next_buffer;

            /* Pick the next buffer from the list. */
            next_buffer = next_buffer->next;

            /* Remove this buffer from the buffer list. */
            OS_ASSERT(sll_remove(&fragment->buffer_list, tmp_buffer, OFFSETOF(FS_BUFFER, next)) != tmp_buffer);

            /* Add this buffer back to the buffer list. */
            fs_buffer_add(tmp_buffer->fd, tmp_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }

        else
        {
            /* There is a hole in the fragment list. */
            status = NET_BUFFER_CONSUMED;

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
            /* Return the buffer we have constructed. */
            *buffer = last_buffer;

            /* Remove condition for this fragment. */
            net_condition_remove(&fragment->condition);

            /* Reset the fragment. */
            ipv4_fragment_reset(fragment);
        }
        else
        {
            /* We still need more data to process this packet. */
            status = NET_BUFFER_CONSUMED;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* ipv4_frag_merge */

#endif /* IPV4_ENABLE_FRAG */

#endif /* NET_IPV4 */
#endif /* CONFIG_NET */
