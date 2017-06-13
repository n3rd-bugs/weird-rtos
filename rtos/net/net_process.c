/*
 * net_process.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <os.h>

#ifdef CONFIG_NET
#include <net.h>
#ifdef NET_IPV4
#include <net_ipv4.h>
#endif
#ifdef NET_ARP
#include <ethernet.h>
#include <net_arp.h>
#endif

/*
 * net_buffer_process
 * @buffer: Received networking buffer needed to be processed.
 * @return: A success is returned if the buffer was processed and caller can
 * safely free this buffer, NET_BUFFER_CONSUMED will be returned to the
 * caller that the buffer cannot be freed now.
 * This is will process a given buffer.
 */
int32_t net_buffer_process(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    uint32_t flags;
    uint8_t protocol;

    SYS_LOG_FUNTION_ENTRY(NET_PROCESS);

    /* Skim the protocol and flags from the buffer. */
    OS_ASSERT(fs_buffer_pull(buffer, &flags, sizeof(uint32_t), 0) != SUCCESS);
    OS_ASSERT(fs_buffer_pull(buffer, &protocol, sizeof(uint8_t), 0) != SUCCESS);

    /* Interpret the protocol. */
    /* [TODO] In future this might be controlled by some sort of protocol plugin. */
    switch (protocol)
    {
#ifdef NET_IPV4
    /* IPv4 protocol. */
    case NET_PROTO_IPV4:

        /* Process this IPv4 buffer. */
        status = net_process_ipv4(buffer, flags);

        break;
#endif

#ifdef NET_ARP
    /* ARP packet. */
    case NET_PROTO_ARP:

        /* Process this ARP buffer. */
        status = net_process_arp(buffer);

        break;
#endif
    default:
        break;
    }

    SYS_LOG_FUNTION_EXIT_STATUS(NET_PROCESS, (status == NET_BUFFER_CONSUMED) ? SUCCESS : status);

    /* Return status to the caller. */
    return (status);

} /* net_buffer_process */

#endif /* CONFIG_NET */
