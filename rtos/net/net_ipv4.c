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
#include <net_ipv4.h>

/* Function prototypes. */
int32_t ipv4_process_ihl(void *, uint8_t *, uint32_t);
int32_t ipv4_process_payload_length(void *, uint8_t *, uint32_t);
int32_t ipv4_process_protocol(void *, uint8_t *, uint32_t);
int32_t ipv4_process_src_address(void *, uint8_t *, uint32_t);
int32_t ipv4_process_dst_address(void *, uint8_t *, uint32_t);

/* IPv4 header list. */
const HEADER ipv4_hdr[] =
{
    /*  Header data                             Size,   Type flags,                     */
    {   0,                                      4,      HEADER_BIT,                     },  /* Version */
    {   &ipv4_process_ihl,                      4,      HEADER_BIT | HEADER_PROCESS,    },  /* Internet header length */
    {   0,                                      1,      0,                              },  /* Type of service */
    {   &ipv4_process_payload_length,           2,      HEADER_PROCESS,                 },  /* Payload length */
    {   0,                                      2,      0,                              },  /* Identification */
    {   0,                                      3,      HEADER_BIT,                     },  /* Flags */
    {   0,                                      13,     HEADER_BIT,                     },  /* Fragment offset */
    {   0,                                      1,      0,                              },  /* Time to live */
    {   &ipv4_process_protocol,                 1,      HEADER_PROCESS,                 },  /* Protocol */
    {   0,                                      2,      0,                              },  /* Checksum */
    {   &ipv4_process_src_address,              4,      HEADER_PROCESS,                 },  /* Source address */
    {   &ipv4_process_dst_address,              4,      HEADER_PROCESS,                 },  /* Destination address */
    {   0,                                      3,      0,                              },  /* Options */
    {   0,                                      1,      0,                              },  /* Padding */
    {   0,                                      0,      HEADER_END,                     },  /* --- */
};


/*
 * net_process_ipv4
 * @buffer: File system buffer needed to be processed.
 * This function will process an incoming IPv4 packet.
 */
int32_t net_process_ipv4(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    HDR_MACHINE machine;
    uint8_t proc_buffer[5];

    /* Initialize header machine. */
    header_machine_init(&machine, &fs_buffer_hdr_pull);

    /* Parse the header. */
    header_machine_run(&machine, NULL, ipv4_hdr, buffer, proc_buffer);

    /* Return status to the caller. */
    return (status);

} /* net_process_ipv4 */

/*
 * ipv4_process_ihl
 * @buffer: File system buffer needed to be processed.
 * This function will process an incoming IPv4 packet.
 */
int32_t ipv4_process_ihl(void *data, uint8_t *value, uint32_t length)
{
    UNUSED_PARAM(data);
    UNUSED_PARAM(value);
    UNUSED_PARAM(length);

    return (SUCCESS);

} /* ipv4_process_ihl */

int32_t ipv4_process_payload_length(void *data, uint8_t *value, uint32_t length)
{
    UNUSED_PARAM(data);
    UNUSED_PARAM(value);
    UNUSED_PARAM(length);

    return (SUCCESS);

} /* ipv4_process_payload_length */

int32_t ipv4_process_protocol(void *data, uint8_t *value, uint32_t length)
{
    UNUSED_PARAM(data);
    UNUSED_PARAM(value);
    UNUSED_PARAM(length);

    return (SUCCESS);

} /* ipv4_process_protocol */

int32_t ipv4_process_src_address(void *data, uint8_t *value, uint32_t length)
{
    UNUSED_PARAM(data);
    UNUSED_PARAM(value);
    UNUSED_PARAM(length);

    return (SUCCESS);

} /* ipv4_process_src_address */

int32_t ipv4_process_dst_address(void *data, uint8_t *value, uint32_t length)
{
    UNUSED_PARAM(data);
    UNUSED_PARAM(value);
    UNUSED_PARAM(length);

    return (SUCCESS);

} /* ipv4_process_dst_address */

#endif /* CONFIG_NET */
