/*
 * net_icmp.c
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

#ifdef NET_ICMP
#include <fs.h>
#include <fs_buffer.h>
#include <net_icmp.h>
#include <net_csum.h>

/*
 * icmp_header_add
 * @buffer: File system buffer on which ICMP header is needed to be added.
 * @type: ICMP type.
 * @code: ICMP type code.
 * This function will add an ICMP header on the given buffer.
 */
int32_t icmp_header_add(FS_BUFFER *buffer, uint8_t type, uint8_t code)
{
    int32_t status = SUCCESS;
    uint16_t csum = 0;

    /* Push the ICMP header. */

    /* We already have the payload in the buffer. */

    /* Add trailing data. */
    status = fs_buffer_push(buffer, ((uint8_t []){0, 0, 0, 0}), 4, FS_BUFFER_HEAD);

    if (status == SUCCESS)
    {
        /* Add 0 checksum. */
        status = fs_buffer_push(buffer, &csum, 2, FS_BUFFER_HEAD);
    }

    if (status == SUCCESS)
    {
        /* Add ICMP code. */
        status = fs_buffer_push(buffer, &code, 1, FS_BUFFER_HEAD);
    }

    if (status == SUCCESS)
    {
        /* Add ICMP type. */
        status = fs_buffer_push(buffer, &type, 1, FS_BUFFER_HEAD);
    }


    if (status == SUCCESS)
    {
        /* Compute and update the value of checksum field. */
        csum = net_csum_calculate(buffer);
        status = fs_buffer_push_offset(buffer, &csum, 2, ICMP_HDR_CSUM_OFFSET, (FS_BUFFER_HEAD | FS_BUFFER_UPDATE));
    }

    /* Return status to the caller. */
    return (status);

} /* icmp_header_add */

#endif /* NET_ICMP */
#endif /* CONFIG_NET */
