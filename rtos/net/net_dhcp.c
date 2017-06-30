/*
 * net_dhcp.c
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
#include <kernel.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_DHCP
#include <ethernet.h>
#include <net_dhcp.h>
#include <header.h>

/*
 * dhcp_add_header
 * @buffer: File system buffer needed to be populated.
 * @operation: BOOTP operation.
 * @xid: Transaction ID.
 * @seconds: Time passed since this transaction was started.
 * @bcast: If TRUE broadcast flag will be set otherwise this is a unicast.
 * @client_address: Client IPv4 address.
 * @your_address: Your IPv4 address.
 * @server_address: Server IPv4 address.
 * @hw_address: Ethernet address for client.
 * @return: A success status will be returned if DHCP header was successfully
 *  added to the given buffer.
 * This function will add the given DHCP header on the provided buffer.
 */
int32_t dhcp_add_header(FS_BUFFER *buffer, uint8_t operation, uint32_t xid, uint16_t seconds, uint8_t bcast, uint32_t client_address, uint32_t your_address, uint32_t server_address, uint8_t *hw_address)
{
    int32_t status;
    uint32_t magic_cookie = DHCP_MAGIC_COKIE, op_word = (((uint32_t)operation << DHCP_HDR_OP_SHIFT) | DHCP_OP_HEADER), padding[4] = {0, 0, 0, 0}; /* 16 byte padding size. */
    uint16_t flags = (uint16_t)(bcast << DHCP_HDR_BCAST_SHIFT);
    uint8_t num_paddings = 12;
    HDR_GEN_MACHINE machine;
    HEADER headers[] =
    {
        {&op_word,                          4,                                      (FS_BUFFER_PACKED) },   /* Operation, hardware type/hardware address length, hop limit. */
        {&xid,                              4,                                      0 },                    /* Transaction ID. */
        {&seconds,                          2,                                      (FS_BUFFER_PACKED) },   /* Transaction life time. */
        {&flags,                            2,                                      (FS_BUFFER_PACKED) },   /* Flags. */
        {&client_address,                   IPV4_ADDR_LEN,                          (FS_BUFFER_PACKED) },   /* Client IP address. */
        {&your_address,                     IPV4_ADDR_LEN,                          (FS_BUFFER_PACKED) },   /* Your IP address. */
        {&server_address,                   IPV4_ADDR_LEN,                          (FS_BUFFER_PACKED) },   /* Server IP address. */
        {(uint32_t []){IPV4_ADDR_UNSPEC},   IPV4_ADDR_LEN,                          (FS_BUFFER_PACKED) },   /* Relay agent address. */
        {hw_address,                        ETH_ADDR_LEN,                           0 },                    /* Ethernet address. */
        {padding,                           (DHCP_HDR_CHADDR_LEN - ETH_ADDR_LEN),   0 },                    /* Address padding. */
    };

    SYS_LOG_FUNCTION_ENTRY(DHCP);

    /* Append magic cookie on the buffer. */
    status = fs_buffer_hdr_push(buffer, (uint8_t *)&magic_cookie, 4, FS_BUFFER_PACKED);

    /* Add 192-bytes of 0-paddings on the buffer. */
    while ((status == SUCCESS) && (num_paddings --))
    {
        /* Append a 16-byte 0-padding. */
        status = fs_buffer_hdr_push(buffer, (uint8_t *)padding, 16, 0);
    }

    /* If magic number and paddings were successfully added to the buffer. */
    if (status == SUCCESS)
    {
        /* Initialize header machine. */
        header_gen_machine_init(&machine, &fs_buffer_hdr_push);

        /* Push DHCP header on the buffer. */
        status = header_generate(&machine, headers, sizeof(headers)/sizeof(HEADER), buffer);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(DHCP, status);

    /* Return status to the caller. */
    return (status);

} /* dhcp_add_header */

/*
 * dhcp_get_header
 * @buffer: File system buffer needed to be parsed.
 * @operation: BOOTP parsed will be returned here.
 * @xid: Transaction ID.
 * @client_address: Client IPv4 address.
 * @your_address: Your IPv4 address.
 * @server_address: Server IPv4 address.
 * @hw_address: Ethernet address parsed.
 * @return: A success status will be returned if DHCP header was successfully
 *  parsed from the given buffer.
 * This function will parse the DHCP header from the given buffer.
 */
int32_t dhcp_get_header(FS_BUFFER *buffer, uint8_t *operation, uint32_t *xid, uint32_t *client_address, uint32_t *your_address, uint32_t *server_address, uint8_t *hw_address)
{
    int32_t status;
    uint32_t op_word;
    HDR_PARSE_MACHINE machine;
    HEADER headers[] =
    {
        {&op_word,          4,                                              (FS_BUFFER_PACKED) },   /* Operation, hardware type/hardware address length, hop limit. */
        {xid,               4,                                              0 },                    /* Transaction ID. */
        {NULL,              4,                                              0 },                    /* Transaction life time, flags. */
        {client_address,    IPV4_ADDR_LEN,                                  (FS_BUFFER_PACKED) },   /* Client IP address. */
        {your_address,      IPV4_ADDR_LEN,                                  (FS_BUFFER_PACKED) },   /* Your IP address. */
        {server_address,    IPV4_ADDR_LEN,                                  (FS_BUFFER_PACKED) },   /* Server IP address. */
        {NULL,              IPV4_ADDR_LEN,                                  0 },                    /* Relay agent address. */
        {hw_address,        ETH_ADDR_LEN,                                   0 },                    /* Ethernet address. */
        {NULL,              (DHCP_HDR_CHADDR_LEN - ETH_ADDR_LEN) + 4 + 192, 0 },                    /* Address padding, server name, file name, DHCP magic number. */
    };

    SYS_LOG_FUNCTION_ENTRY(DHCP);

    /* Initialize a header parse machine. */
    header_parse_machine_init(&machine, &fs_buffer_hdr_pull);

    /* Try to parse the DHCP header from the packet. */
    status = header_parse(&machine, headers, sizeof(headers)/sizeof(HEADER), buffer);

    /* If DHCP header was successfully parsed. */
    if (status == SUCCESS)
    {
        /* Return the operation parsed from the header. */
        *operation = (uint8_t)(op_word >> DHCP_HDR_OP_SHIFT);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(DHCP, status);

    /* Return status to the caller. */
    return (status);

} /* dhcp_get_header */

/*
 * dhcp_add_option
 * @buffer: File system buffer needed to be populated.
 * @type: DHCP option type.
 * @length: Length of option value.
 * @data: Option value.
 * @flags: Option value flags, FS_BUFFER_PACKED if this is a packed option.
 * @return: A success status will be returned if DHCP header was successfully
 * added to the given buffer.
 * This function will add the given DHCP header on the provided buffer.
 */
int32_t dhcp_add_option(FS_BUFFER *buffer, uint8_t type, uint8_t length, void *value, uint8_t flags)
{
    int32_t status = SUCCESS;

    SYS_LOG_FUNCTION_ENTRY(DHCP);

    /* Append option type. */
    status = fs_buffer_push(buffer, (uint8_t *)&type, 1, 0);

    /* If we do need to add option value. */
    if ((status == SUCCESS) && (type != DHCP_OPT_END))
    {
        /* Append option length. */
        status = fs_buffer_push(buffer, (uint8_t *)&length, 1, 0);

        if (status == SUCCESS)
        {
            /* Append option value. */
            status =  fs_buffer_push(buffer, (uint8_t *)value, length, flags);
        }
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(DHCP, status);

    /* Return status to the caller. */
    return (status);

} /* dhcp_add_option */

/*
 * dhcp_get_option
 * @buffer: File system buffer from which option will be parsed.
 * @type: DHCP option type will be returned here.
 * @length: DHCP option value length will be returned here.
 * @return: A success status will be returned if a DHCP option was successfully
 *  parsed.
 * This function will parse a DHCP option from the given buffer, the option
 * value will remain on the start of buffer and caller is responsible for
 * pulling it from the buffer.
 */
int32_t dhcp_get_option(FS_BUFFER *buffer, uint8_t *type, uint8_t *length)
{
    int32_t status = SUCCESS;

    SYS_LOG_FUNCTION_ENTRY(DHCP);

    /* Pull option type. */
    status = fs_buffer_pull(buffer, type, 1, 0);

    /* If we do have some option value next. */
    if ((status == SUCCESS) && (*type != DHCP_OPT_END))
    {
        /* Pull option length. */
        status = fs_buffer_pull(buffer, length, 1, 0);
    }

    SYS_LOG_FUNCTION_EXIT_STATUS(DHCP, status);

    /* Return status to the caller. */
    return (status);

} /* dhcp_get_option */

#endif /* NET_DHCP */

#endif /* CONFIG_NET */
