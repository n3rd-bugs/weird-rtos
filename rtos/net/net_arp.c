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
#include <ethernet.h>
#include <header.h>

/* Internal function prototypes. */
static int32_t arp_process_prologue_ipv4(FS_BUFFER *);
static int32_t arp_process_request(FS_BUFFER *);
static int32_t arp_send_response(FS_BUFFER *, uint8_t *, uint32_t, uint8_t *, uint32_t);
static ARP_ENTRY *arp_find_entry(FD, uint32_t);

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
        status = arp_send_response(buffer, ethernet_get_mac_address(buffer->fd), own_ip, dst_mac, target_ip);
    }

    /* Return status to the caller. */
    return (status);

} /* arp_process_request */

/*
 * arp_send_response
 * @buffer: Buffer needed to send.
 * @src_mac: Source MAC address in ARP header.
 * @src_ip: Source IPv4 address in ARP header.
 * @dst_mac: Destination MAC address.
 * @dst_ip: Destination IP address.
 * @return: A success status will be returned if response was successfully sent,
 *  NET_BUFFER_CONSUMED will be returned if buffer was consumed and we don't
 *  need to free it.
 * This function process an ARP request and sends a reply if needed.
 */
static int32_t arp_send_response(FS_BUFFER *buffer, uint8_t *src_mac, uint32_t src_ip, uint8_t *dst_mac, uint32_t dst_ip)
{
    int32_t status;
    HDR_GEN_MACHINE machine;
    HEADER headers[] =
    {
        {(uint16_t []){ARP_ETHER_TYPE},     2,              (FS_BUFFER_PACKED) },   /* Hardware type. */
        {(uint16_t []){ARP_PROTO_IP},       2,              (FS_BUFFER_PACKED) },   /* Protocol type. */
        {(uint8_t []){ETH_ADDR_LEN},        1,              0 },                    /* Hardware address length. */
        {(uint8_t []){IPV4_ADDR_LEN},       1,              0 },                    /* Protocol address length. */
        {(uint16_t []){ARP_OP_RESPONSE},    2,              (FS_BUFFER_PACKED) },   /* ARP operation. */
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

} /* arp_send_response */

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

                /* Not implemented yet. */
                OS_ASSERT(TRUE);

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
    ARP_DATA *data = ethernet_arp_get_data(fd);
    ARP_ENTRY *entry = NULL;
    uint32_t clock = current_system_tick();
    uint32_t i;

    /* Go though all the ARP entries in the device. */
    for (i = 0; i < data->num_entries; i++)
    {
        /* If this required entry. */
        if (data->entries[i].ip == address)
        {
            /* Return this ARP entry. */
            entry = &data->entries[i];

            /* Break out of this loop. */
            break;
        }

        /* Check if this entry is free, or we can reuse this ARP entry. */
        else if ((entry != NULL) && (((data->entries[i].flags & ARP_FLAG_VALID) == 0) || ((clock - data->entries[i].birth_time) > ARP_REUSE_LIFE_TIME)))
        {
            /* Save this APR entry as it can be reused. */
            entry = data->entries[i];
        }
    }

    /* Return required ARP entry. */
    return (entry);

} /* arp_find_entry */

/*
 * arp_resolve
 * @buffer: IPv4 packet for which a destination ethernet address is needed to
 *  be resolved.
 * This function will try to resolve the destination ethernet address for an
 * IPv4 packet.
 */
int32_t arp_resolve(FS_BUFFER *buffer, uint8_t **dst_addr)
{
    int32_t status = SUCCESS;
    uint32_t dst_ip;
    ARP_ENTRY *entry;

    /* Pull the intended destination IP address. */
    OS_ASSERT(fs_buffer_pull_offset(buffer, &dst_ip, IPV4_ADDR_LEN, IPV4_HDR_DST_OFFSET, FS_BUFFER_INPLACE) != SUCCESS);

    /* Try to find an entry in ARP cache for the device. */
    entry = arp_find_entry(buffer->fd, dst_ip);

    /* If we do have an entry. */
    if (entry != NULL)
    {
        /* If destination is reachable. */
        if (entry->flags & ARP_FLAG_UP)
        {
            /* Return the destination MAC address to be used. */
            *dst_addr = entry->mac;
        }
        else
        {
            /* Check if this is a new entry. */
            if ((entry->flags & ARP_FLAG_VALID) == 0)
            {
                /* Initialize this ARP entry. */
                entry->flags |= ARP_FLAG_VALID;
                entry->ip = dst_ip;
            }

            /* Put this buffer in the ARP buffer list. */
            sll_append(&entry->buffer_list, buffer, OFFSETOF(FS_BUFFER, next));
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

#endif /* NET_ARP */
#endif /* CONFIG_NET */
