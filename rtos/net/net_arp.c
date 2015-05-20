/*
 * net_arp.c
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

#ifdef NET_ARP
#include <string.h>
#include <ethernet.h>
#include <header.h>
#include <sll.h>

/* Internal function prototypes. */
static int32_t arp_process_prologue_ipv4(FS_BUFFER *);
static int32_t arp_process_request(FS_BUFFER *);
static int32_t arp_process_response(FS_BUFFER *);
static int32_t arp_send_packet(FS_BUFFER *, uint16_t, uint8_t *, uint32_t, uint8_t *, uint32_t);
static void arp_free_entry(ARP_ENTRY *);
static ARP_ENTRY *arp_find_entry(FD, uint32_t);
static void arp_update_timers(FD);
static int32_t arp_route(FD, ARP_ENTRY *);
static void arp_event(void *);

/*
 * arp_process_prologue_ipv4
 * @buffer: An ARP buffer needed to be processed.
 * @return: A success status will be returned if this ARP packet looks okay
 *  and we can process it for the given interface, NET_INVALID_HDR will be
 *  returned if an invalid header was parsed.
 * This function processes prologue for a given ARP packet and verify that this
 * is for IPv4 running over ethernet.
 */
static int32_t arp_process_prologue_ipv4(FS_BUFFER *buffer)
{
    int32_t status;
    HDR_PARSE_MACHINE hdr_machine;
    uint16_t hardtype, prototype;
    uint8_t hardlen, protolen;
    HEADER headers[] =
    {
        {(uint8_t *)&hardtype,      2,              (FS_BUFFER_PACKED) },   /* Hardware type. */
        {(uint8_t *)&prototype,     2,              (FS_BUFFER_PACKED) },   /* Protocol type. */
        {&hardlen,                  1,              0 },                    /* Hardware address length. */
        {&protolen,                 1,              0 },                    /* Protocol address length. */
    };

    /* Initialize a header parse machine. */
    header_parse_machine_init(&hdr_machine, &fs_buffer_hdr_pull);

    /* Try to parse the prologue header from the packet. */
    status = header_parse(&hdr_machine, headers, sizeof(headers)/sizeof(HEADER), buffer);

    /* If ARP prologue was successfully parsed. */
    if (status == SUCCESS)
    {
        /* Verify that we have ethernet header type, IPv4 protocol and address
         * size fields are correct. */
        if ((hardtype != ARP_ETHER_TYPE) || (prototype != ARP_PROTO_IP) || (hardlen != ETH_ADDR_LEN) || (protolen != IPV4_ADDR_LEN))
        {
            /* This either not supported of an invalid header was parsed. */
            status = NET_INVALID_HDR;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* arp_process_prologue_ipv4 */

/*
 * arp_process_request
 * @buffer: An ARP request buffer needed to be processed.
 * @return: A success status will be returned if packet was successfully parsed,
 *  NET_BUFFER_CONSUMED will be returned if buffer was consumed and we don't
 *  need to free it.
 * This function process an ARP request and sends a reply if needed.
 */
static int32_t arp_process_request(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    uint32_t own_ip, target_ip;
    uint8_t dst_mac[ETH_ADDR_LEN];

    /* Pull the address required by the caller. */
    OS_ASSERT(fs_buffer_pull_offset(buffer, &target_ip, IPV4_ADDR_LEN, ARP_HDR_TGT_IPV4_OFFSET, (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)) != SUCCESS);

    /* Get IPv4 address assigned to the device on which we have received this
     * packet. */
    OS_ASSERT(ipv4_get_device_address(buffer->fd, &own_ip) != SUCCESS);

    /* If remote needs our target hardware address. */
    if (own_ip == target_ip)
    {
        /* Pull the IPv4 address to which we need to send this a response. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &target_ip, IPV4_ADDR_LEN, ARP_HDR_SRC_IPV4_OFFSET, (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)) != SUCCESS);

        /* Pull the ethernet address to which we need to send this a response. */
        OS_ASSERT(fs_buffer_pull_offset(buffer, &dst_mac, IPV4_ADDR_LEN, ARP_HDR_SRC_HW_OFFSET, (FS_BUFFER_INPLACE)) != SUCCESS);

        /* Pull and discard any data still on this buffer. */
        OS_ASSERT(fs_buffer_pull(buffer, NULL, buffer->total_length, 0) != SUCCESS);

        /* Send response for this ARP request. */
        status = arp_send_packet(buffer, ARP_OP_RESPONSE, ethernet_get_mac_address(buffer->fd), own_ip, dst_mac, target_ip);
    }

    /* Return status to the caller. */
    return (status);

} /* arp_process_request */

/*
 * arp_process_response
 * @buffer: An ARP response buffer needed to be processed.
 * @return: A success status will be returned if packet was successfully parsed,
 *  NET_BUFFER_CONSUMED will be returned if buffer was consumed and we don't
 *  need to free it.
 * This function process an ARP response packet.
 */
static int32_t arp_process_response(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    uint32_t src_ip, i;
    ARP_DATA *arp_data = ethernet_arp_get_data(buffer->fd);

    /* Pull the IPv4 address sent by the remote. */
    OS_ASSERT(fs_buffer_pull_offset(buffer, &src_ip, IPV4_ADDR_LEN, ARP_HDR_SRC_IPV4_OFFSET, (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)) != SUCCESS);

    /* Go though all the ARP entries in the device. */
    for (i = 0; i < arp_data->num_entries; i++)
    {
        /* Check if this is the entry for which we have received a response. */
        if (arp_data->entries[i].ip == src_ip)
        {
            /* Set this entry as up. */
            arp_data->entries[i].flags |= ARP_FLAG_UP;

            /* Send any packets that are still needed to be sent. */
            if (arp_data->entries[i].buffer_list.head != NULL)
            {
                /* ARP will only accumulate IPv4 packets, so try to send them again. */
                net_device_buffer_transmit(arp_data->entries[i].buffer_list.head, NET_PROTO_IPV4, 0);

                /* Clear the ARP buffer list. */
                arp_data->entries[i].buffer_list.head = arp_data->entries[i].buffer_list.tail = NULL;
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* arp_process_response */

/*
 * arp_send_packet
 * @buffer: Buffer needed to send.
 * @operation: ARP operation needed to be performed.
 * @src_mac: Source MAC address in ARP header.
 * @src_ip: Source IPv4 address in ARP header.
 * @dst_mac: Destination MAC address.
 * @dst_ip: Destination IP address.
 * @return: A success status will be returned if response was successfully sent,
 *  NET_BUFFER_CONSUMED will be returned if buffer was consumed and we don't
 *  need to free it.
 * This function process an ARP request and sends a reply if needed.
 */
static int32_t arp_send_packet(FS_BUFFER *buffer, uint16_t operation, uint8_t *src_mac, uint32_t src_ip, uint8_t *dst_mac, uint32_t dst_ip)
{
    int32_t status;
    HDR_GEN_MACHINE machine;
    HEADER headers[] =
    {
        {(uint16_t []){ARP_ETHER_TYPE},     2,              (FS_BUFFER_PACKED) },   /* Hardware type. */
        {(uint16_t []){ARP_PROTO_IP},       2,              (FS_BUFFER_PACKED) },   /* Protocol type. */
        {(uint8_t []){ETH_ADDR_LEN},        1,              0 },                    /* Hardware address length. */
        {(uint8_t []){IPV4_ADDR_LEN},       1,              0 },                    /* Protocol address length. */
        {&operation,                        2,              (FS_BUFFER_PACKED) },   /* ARP operation. */
        {src_mac,                           ETH_ADDR_LEN,   0 },                    /* Source HW address. */
        {&src_ip,                           IPV4_ADDR_LEN,  (FS_BUFFER_PACKED) },   /* Source IPv4 address. */
        {dst_mac,                           ETH_ADDR_LEN,   0 },                    /* Destination HW address. */
        {&dst_ip,                           IPV4_ADDR_LEN,  (FS_BUFFER_PACKED) },   /* Destination IPv4 address. */
    };

    /* Initialize header machine. */
    header_gen_machine_init(&machine, &fs_buffer_hdr_push);

    /* Add required ARP header. */
    header_generate(&machine, headers, sizeof(headers)/sizeof(HEADER), buffer);

    /* Send an ARP packet on the device. */
    status = net_device_buffer_transmit(buffer, NET_PROTO_ARP, 0);

    /* Return status to the caller. */
    return (status);

} /* arp_send_packet */

/*
 * arp_free_entry
 * @entry: ARP entry needed to be freed.
 * This function will free an ARP entry.
 */
static void arp_free_entry(ARP_ENTRY *entry)
{
    /* Free any buffers still on this entry. */
    if (entry->buffer_list.head != NULL)
    {
        /* Free all the buffers still on this ARP entry. */
        fs_buffer_add_buffer_list(entry->buffer_list.head, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

        /* Clear the buffer list. */
        entry->buffer_list.head = entry->buffer_list.tail = NULL;
    }

    /* Clear the ARP entry flags to reinitialize this entry. */
    entry->flags = 0;

} /* arp_free_entry */

/*
 * arp_find_entry
 * @fd: Ethernet device descriptor from which an entry is required.
 * @address: IPv4 address for which ARP entry is required.
 * @return: If not null either a new entry or an existing entry will be
 *  returned, caller will need to see the entry flags for it's state. If null
 *  no existing entry was found and there is no free ARP entry.
 * This function will try to find an existing ARP entry for the required
 * destination, if found will be returned to the caller, otherwise a free entry
 * will be returned if available.
 */
static ARP_ENTRY *arp_find_entry(FD fd, uint32_t address)
{
    /* Get ARP data for this device. */
    ARP_DATA *arp_data = ethernet_arp_get_data(fd);
    ARP_ENTRY *entry = NULL;
    uint32_t i, clock = (uint32_t)current_system_tick();

    /* Go though all the ARP entries in the device. */
    for (i = 0; i < arp_data->num_entries; i++)
    {
        /* If this required entry. */
        if (arp_data->entries[i].ip == address)
        {
            /* Return this ARP entry. */
            entry = &arp_data->entries[i];

            /* Break out of this loop. */
            break;
        }

        /* Check if this entry is free, or we can reuse this ARP entry. */
        else if ((entry == NULL) && (((arp_data->entries[i].flags & ARP_FLAG_VALID) == 0) || ((clock - arp_data->entries[i].birth_time) > ARP_REUSE_LIFE_TIME)))
        {
            /* Save this APR entry as it can be reused. */
            entry = &arp_data->entries[i];
        }
    }

    /* Return required ARP entry. */
    return (entry);

} /* arp_find_entry */

/*
 * arp_update_timers
 * @fd: Ethernet device file descriptor.
 * This function will update the timer values for ARP.
 */
static void arp_update_timers(FD fd)
{
    /* Get ARP data for this device. */
    ARP_DATA *arp_data = ethernet_arp_get_data(fd);
    uint32_t i, this_timeout, next_timeout = MAX_WAIT, clock = (uint32_t)current_system_tick();

    /* Go though all the ARP entries in the device. */
    for (i = 0; i < arp_data->num_entries; i++)
    {
        /* Check if we have a still to be processed entry. */
        if ((arp_data->entries[i].flags & ARP_FLAG_VALID) && ((arp_data->entries[i].flags & ARP_FLAG_UP) == 0))
        {
            /* Calculate the next timeout for this entry. */
            this_timeout = (((uint32_t)(arp_data->entries[i].retry_count * ARP_RETRY_COUNT) + arp_data->entries[i].birth_time) - clock);

            /* If this time out is smaller then the one we have previously
             * calculated. */
            if (this_timeout < next_timeout)
            {
                /* Use this timeout. */
                next_timeout = this_timeout;
            }
        }
    }

    /* If we do have a timeout that we need to process. */
    if (next_timeout != MAX_WAIT)
    {
        /* Save the timeout at which we will need to process next ARP event. */
        arp_data->suspend.timeout = next_timeout;

        /* Add networking condition to process ARP for this device. */
        net_condition_add(&arp_data->condition, &arp_data->suspend, &arp_event, fd);
    }

} /* arp_update_timers */

/*
 * arp_route
 * @fd: Ethernet device file descriptor.
 * @entry: ARP entry for which route is needed to be resolved.
 * This function will start the process of ARP routing and sends the first
 * ARP request.
 */
static int32_t arp_route(FD fd, ARP_ENTRY *entry)
{
    int32_t status = SUCCESS;
    FS_BUFFER *buffer;
    uint32_t src_ip;

    /* Get IPv4 address assigned to this device. */
    OS_ASSERT(ipv4_get_device_address(fd, &src_ip) != SUCCESS);

    /* Get a free buffer that can be used to send an ARP request. */
    buffer = fs_buffer_get(fd, FS_BUFFER_LIST, 0);

    if (buffer != NULL)
    {
        /* Send an ARP request. */
        status = arp_send_packet(buffer, ARP_OP_REQUEST, ethernet_get_mac_address(fd), src_ip, entry->mac, entry->ip);

        /* If request was successfully sent. */
        if (status == NET_BUFFER_CONSUMED)
        {
            /* Reset the status. */
            status = SUCCESS;
        }
    }
    else
    {
        /* No buffer to send a request. */
        status = NET_NO_BUFFERS;
    }

    /* Return status to the caller. */
    return (status);

} /* arp_route */

/*
 * arp_event
 * @data: Ethernet device file descriptor.
 * This function will process an ARP event.
 */
static void arp_event(void *data)
{
    FD fd = (FD)data;
    ARP_DATA *arp_data = ethernet_arp_get_data(fd);
    uint32_t i, clock = (uint32_t)current_system_tick();

    /* Acquire lock for this file descriptor. */
    OS_ASSERT(fd_get_lock(fd) != SUCCESS);

    /* Go though all the ARP entries in the device. */
    for (i = 0; i < arp_data->num_entries; i++)
    {
        /* Check if we have sent maximum number of ARP requests for this ARP
         * entry. */
        if (arp_data->entries[i].retry_count == ARP_RETRY_COUNT)
        {
            /* Free this ARP entry. */
            arp_free_entry(&arp_data->entries[i]);
        }

        /* Check if we have a still to be processed entry. */
        else if ((arp_data->entries[i].flags & ARP_FLAG_VALID) && ((arp_data->entries[i].flags & ARP_FLAG_UP) == 0))
        {
            /* Check if we need to send a new request for this ARP entry. */
            if (((uint32_t)(arp_data->entries[i].retry_count * ARP_RETRY_COUNT) + arp_data->entries[i].birth_time) <= clock)
            {
                /* Try to find route for this entry. */
                OS_ASSERT(arp_route(fd, &arp_data->entries[i]) != SUCCESS);

                /* Increment the retry count for this ARP entry. */
                arp_data->entries[i].retry_count++;
            }
        }
    }

    /* Remove this ARP condition from networking stack. */
    net_condition_remove(&arp_data->condition);

    /* Update ARP timers. */
    arp_update_timers(fd);

    /* Release lock for this file descriptor. */
    fd_release_lock(fd);

} /* arp_event */

/*
 * arp_resolve
 * @buffer: IPv4 packet for which a destination ethernet address is needed to
 *  be resolved.
 * @dst_addr: Destination MAC address will be copied in here if found.
 * This function will try to resolve the destination ethernet address for an
 * IPv4 packet.
 */
int32_t arp_resolve(FS_BUFFER *buffer, uint8_t *dst_addr)
{
    int32_t status = SUCCESS;
    uint32_t dst_ip;
    ARP_ENTRY *entry;

    /* Pull the intended destination IP address. */
    OS_ASSERT(fs_buffer_pull_offset(buffer, &dst_ip, IPV4_ADDR_LEN, IPV4_HDR_DST_OFFSET, (FS_BUFFER_PACKED | FS_BUFFER_INPLACE)) != SUCCESS);

    /* Try to find an entry in ARP cache for the device. */
    entry = arp_find_entry(buffer->fd, dst_ip);

    /* If we do have an entry. */
    if (entry != NULL)
    {
        /* If we are using an expired entry. */
        if ((entry->flags & ARP_FLAG_VALID) && (entry->ip != dst_ip))
        {
            /* Free this ARP entry. */
            arp_free_entry(entry);
        }

        /* If destination is reachable. */
        if (entry->flags & ARP_FLAG_UP)
        {
            /* Return the destination MAC address to be used. */
            memcpy(dst_addr, entry->mac, ETH_ADDR_LEN);
        }
        else
        {
            /* Check if this is a new entry. */
            if ((entry->flags & ARP_FLAG_VALID) == 0)
            {
                /* Clear this ARP entry. */
                memset(entry, 0, sizeof(ARP_ENTRY));

                /* Initialize this ARP entry. */
                entry->flags |= ARP_FLAG_VALID;
                entry->ip = dst_ip;
                entry->birth_time = (uint32_t)current_system_tick();
            }

            /* Put this buffer in the ARP buffer list. */
            sll_append(&entry->buffer_list, buffer, OFFSETOF(FS_BUFFER, next));

            /* Start the ARP timer to start routing for the destination address. */
            arp_update_timers(buffer->fd);

            /* This packet will be sent when we have resolved the destination
             * MAC address. */
            status = NET_BUFFER_CONSUMED;
        }
    }
    else
    {
        /* Destination is not reachable, return an error. */
        status = NET_DST_UNREACHABLE;
    }

    /* Return status to the caller. */
    return (status);

} /* arp_resolve */

/*
 * net_process_arp
 * @buffer: An ARP packet needed to be received and processed.
 * @return: A success status will be returned if buffer was successfully parsed
 *  and processed, NET_BUFFER_CONSUMED will be returned if buffer was consumed
 *  and we don't need to free it, NET_INVALID_HDR will be returned if an
 *  invalid header was parsed.
 * This function will receive and process a given ARP packet.
 */
int32_t net_process_arp(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    uint16_t operation;

    /* If we have valid length in the packet. */
    if (buffer->total_length > ARP_HDR_LEN)
    {
        /* Pull padding from the buffer. */
        OS_ASSERT(fs_buffer_pull(buffer, NULL, (buffer->total_length - ARP_HDR_LEN), FS_BUFFER_TAIL) != SUCCESS);
    }
    else
    {
        /* This is not a valid header. */
        status = NET_INVALID_HDR;
    }

    /* If packet length was verified. */
    if (status == SUCCESS)
    {
        /* Parse prologue of this APR packet. */
        status = arp_process_prologue_ipv4(buffer);

        /* If prologue was successfully parsed. */
        if (status == SUCCESS)
        {
            /* Pull the ARP operation. */
            OS_ASSERT(fs_buffer_pull(buffer, &operation, 2, FS_BUFFER_PACKED) != SUCCESS);

            /* Process the ARP operation. */
            switch(operation)
            {
            /* This is an ARP request. */
            case ARP_OP_REQUEST:

                /* Process this ARP request. */
                status = arp_process_request(buffer);

                break;

            /* This might be a response to a request we sent. */
            case ARP_OP_RESPONSE:

                /* Process the ARP response. */
                status = arp_process_response(buffer);

                break;

            /* Unknown operation. */
            default:

                /* An invalid header was parsed */
                status = NET_INVALID_HDR;

                break;
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* net_process_arp */

#endif /* NET_ARP */
#endif /* CONFIG_NET */
