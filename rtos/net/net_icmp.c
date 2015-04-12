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
#include <header.h>
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
    HDR_GEN_MACHINE hdr_machine;
    uint16_t csum = 0;
    HEADER headers[] =
    {
        {&type,                     1, 0},  /* ICMP type. */
        {&code,                     1, 0},  /* ICMP code. */
        {&csum,                     2, 0},  /* Checksum. */
        {(uint8_t []){0, 0, 0, 0},  4, 0},  /* Unused data. */
    };

    /* Push the ICMP header. */

    /* We already have the payload in the buffer. */

    /* Initialize header generator machine. */
    header_gen_machine_init(&hdr_machine, &fs_buffer_hdr_push);

    /* Push the ICMP header on the buffer. */
    status = header_generate(&hdr_machine, headers, sizeof(headers)/sizeof(HEADER), buffer);

    if (status == SUCCESS)
    {
        /* Compute and update the value of checksum field. */
        csum = net_csum_calculate(buffer, -1);
        status = fs_buffer_push_offset(buffer, &csum, 2, ICMP_HDR_CSUM_OFFSET, (FS_BUFFER_HEAD | FS_BUFFER_UPDATE));
    }

    /* Return status to the caller. */
    return (status);

} /* icmp_header_add */

#endif /* NET_ICMP */
#endif /* CONFIG_NET */
