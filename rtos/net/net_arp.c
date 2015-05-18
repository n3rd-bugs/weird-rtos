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
static int32_t arp_process_prologue(FS_BUFFER *);

/*
 * arp_process_prologue
 * @buffer: An ARP needed to be processed.
 * @return: A success status will be returned if this ARP packet looks okay
 *  and we can process it for the given interface.
 * This function process prologue for a given ARP packet.
 */
static int32_t arp_process_prologue(FS_BUFFER *buffer)
{
    int32_t status;
    HDR_PARSE_MACHINE hdr_machine;
    uint16_t hardtype, prototype;
    uint8_t hardlen, protolen;
    HEADER arp_pre_hdr[] =
    {
        {(uint8_t *)&hardtype,      2,              (FS_BUFFER_PACKED) },   /* Hardware type. */
        {(uint8_t *)&prototype,     2,              (FS_BUFFER_PACKED) },   /* Protocol type. */
        {&hardlen,                  1,              0 },                    /* Hardware address length. */
        {&protolen,                 1,              0 },                    /* Protocol address length. */
    };

    /* Initialize a header parse machine. */
    header_parse_machine_init(&hdr_machine, &fs_buffer_hdr_pull);

    /* Try to parse the prologue header from the packet. */
    status = header_parse(&hdr_machine, arp_pre_hdr, sizeof(arp_pre_hdr)/sizeof(HEADER), buffer);

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

} /* arp_process_prologue */

/*
 * net_process_arp
 * @buffer: An ARP packet needed to be received and processed.
 * @return: A success status will be returned if buffer was successfully parsed
 *  and processed, NET_BUFFER_CONSUMED will be returned if buffer was consumed
 *  and we don't need to free it.
 * This function will receive and process a given ARP packet.
 */
int32_t net_process_arp(FS_BUFFER *buffer)
{
    ARP_DATA *arp_data;
    int32_t status;

    /* Parse prologue of this APR packet. */
    status = arp_process_prologue(buffer);

    /* If prologue was successfully parsed. */
    if (status == SUCCESS)
    {
        /* Get ARP data for this device. */
        arp_data = ethernet_arp_get_data(buffer->fd);
    }

    /* Return status to the caller. */
    return (status);

} /* net_process_arp */

#endif /* NET_ARP */
#endif /* CONFIG_NET */
