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
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>

#ifdef CONFIG_NET
#include <net.h>
#include <net_ipv4.h>

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
    uint8_t protocol;

    /* Skim the protocol from the buffer. */
    OS_ASSERT(fs_buffer_pull(buffer, (char *)&protocol, sizeof(uint8_t), 0) != SUCCESS);

    /* Interpret the protocol. */
    /* [TODO] In future this might be controlled by some sort of protocol plugin. */
    switch (protocol)
    {
#ifdef NET_IPV4
    /* IPv4 protocol. */
    case NET_PROTO_IPV4:

        /* Process this IPv4 buffer. */
        net_process_ipv4(buffer);

        break;
#endif
    default:
        break;
    }

    /* Return status to the caller. */
    return (status);

} /* net_buffer_process */

#endif /* CONFIG_NET */
