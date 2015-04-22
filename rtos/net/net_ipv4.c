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
#include <sll.h>
#include <string.h>

/* Internal function prototypes. */
#ifdef IPV4_ENABLE_FRAG
static uint8_t ipv4_frag_sort(void *, void *);
static int32_t ipv4_frag_add(FS_BUFFER **, uint16_t);
static int32_t ipv4_frag_merge(IPV4_FRAGMENT *, FS_BUFFER **);
#endif

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
 * @return: A success status will be returned if IPv4 packet was successfully
 *  processed.
 *  NET_BUFFER_CONSUMED will be returned if buffer was consumed and
 *  should not be freed by the caller.
 *  NET_INVALID_HDR will be returned if an invalid header was parsed.
 *  NET_NOT_SUPPORTED will be returned if an unsupported IPv4 header was parsed.
 * This function will process an incoming IPv4 packet.
 */
int32_t net_process_ipv4(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    uint32_t sa = 0, da;
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
        if (net_csum_calculate(buffer, ver_ihl) != 0)
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
#endif /* NET_ICMP */

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
#endif /* NET_ICMP */
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

#ifdef IPV4_ENABLE_FRAG

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
    uint32_t sa;
    int32_t status = NET_BUFFER_CONSUMED, n, index = -1;
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
        if ((net_device->ipv4_fragments[n].flags & IPV4_FRAG_IN_USE) == 0)
        {
            /* Save the index of the free fragment list. */
            index = n;
        }

        /* If we already have a fragment list for this fragment */
        else if ((net_device->ipv4_fragments[n].sa == sa) && (net_device->ipv4_fragments[n].id == id))
        {
            /* Use this fragment list to reassemble the packet. */
            index = n;

            /* Break out of this loop. */
            break;
        }
    }

    /* If do have a fragment list to process this fragment. */
    if (index >= 0)
    {
        /* If this is a new fragment. */
        if ((net_device->ipv4_fragments[index].flags & IPV4_FRAG_IN_USE) == 0)
        {
            /* Initialize this fragment. */
            net_device->ipv4_fragments[index].flags |= IPV4_FRAG_IN_USE;
            net_device->ipv4_fragments[index].id = id;
            net_device->ipv4_fragments[index].sa = sa;
        }

        /* Push this fragment on the fragment list. */
        sll_insert(&net_device->ipv4_fragments[index].buffer_list, *buffer, &ipv4_frag_sort, OFFSETOF(FS_BUFFER, next));

        /* If we have received the last fragment. */
        if (((flag_offset & IPV4_HDR_FALG_MF) == 0) || (net_device->ipv4_fragments[index].flags & IPV4_FRAG_LAST_RCVD))
        {
            /* Set the last fragment received flag anyway. */
            net_device->ipv4_fragments[index].flags |= IPV4_FRAG_LAST_RCVD;
        }

        /* Merge the received fragments on the go. */
        status = ipv4_frag_merge(&net_device->ipv4_fragments[index], buffer);
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

    /* If no holes were found in the fragment list and last fragment has been
     * received. */
    if ((status == SUCCESS) && (fragment->flags & IPV4_FRAG_LAST_RCVD))
    {
        /* Return the buffer we have constructed. */
        *buffer = last_buffer;

        /* Clear the fragment structure. */
        memset(fragment, 0, sizeof(IPV4_FRAGMENT));
    }

    /* Return status to the caller. */
    return (status);

} /* ipv4_frag_merge */

#endif /* IPV4_ENABLE_FRAG */

#endif /* NET_IPV4 */
#endif /* CONFIG_NET */
