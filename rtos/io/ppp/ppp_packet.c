/*
 * ppp_packet.c
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

#ifdef CONFIG_PPP
#include <fs.h>
#include <ppp.h>
#include <ppp_packet.h>

/*
 * ppp_packet_protocol_parse
 * @buffer: Received buffer needed to be parsed.
 * @protocol: Parsed PPP protocol will be returned here.
 * @pfc: If true it means we might receive a protocol with first byte elided.
 * @return: A success status will be returned if header was successfully parsed,
 *  PPP_INVALID_HEADER will be returned if an invalid header was parsed.
 * This function will parse the PPP protocol field in the given buffer and
 * return it's value.
 */
int32_t ppp_packet_protocol_parse(FS_BUFFER *buffer, uint16_t *protocol, uint8_t pfc)
{
    int32_t status = SUCCESS;
    uint16_t ret_protocol;
    uint8_t proto[2];

    /* If we have at-least 1 bytes on the buffer to parse the protocol. */
    if (buffer->total_length >= 1)
    {
        /* Peek the first byte of the buffer. */
        OS_ASSERT(fs_buffer_pull(buffer, proto, 1, FS_BUFFER_INPLACE) != SUCCESS);

        /* Put the first byte of protocol as it is. */
        ret_protocol = proto[0];

        /* If this is even. */
        if (proto[0] & 0x1)
        {
            /* If we have PFC negotiated. */
            if (pfc == TRUE)
            {
                /* First byte of sent protocol is zero and elided. */

                /* Pull and consume the protocol byte. */
                OS_ASSERT(fs_buffer_pull(buffer, NULL, 1, 0) != SUCCESS);
            }

            else
            {
                /* Invalid protocol field was given. */
                status = PPP_INVALID_HEADER;
            }
        }

        /* If first byte is not even then we should have second byte of protocol
         * too. */
        else if (buffer->total_length >= 2)
        {
            /* Peek the first two byte of the buffer. */
            OS_ASSERT(fs_buffer_pull(buffer, &proto, 2, FS_BUFFER_INPLACE) != SUCCESS);

            /* First byte of protocol must be even and second must be odd. */
            if ((!(proto[0] & 0x1)) && (proto[1] & 0x1))
            {
                /* Pull the protocol field. */
                OS_ASSERT(fs_buffer_pull(buffer, &ret_protocol, sizeof(uint16_t), FS_BUFFER_PACKED) != SUCCESS);
            }

            else
            {
                /* Invalid protocol parsed, return an error. */
                status = PPP_INVALID_HEADER;
            }
        }

        else
        {
            /* Invalid protocol parsed, return an error. */
            status = PPP_INVALID_HEADER;
        }
    }

    else
    {
        /* Invalid header, return an error. */
        status = PPP_INVALID_HEADER;
    }

    if (status == SUCCESS)
    {
        /* Return the parsed protocol. */
        *protocol = ret_protocol;
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_packet_protocol_parse */

/*
 * ppp_packet_protocol_add
 * @buffer: Buffer on which PPP protocol is needed to be added.
 * @protocol: PPP protocol needed to be added.
 * @pfc: If true it means we can compress the protocol field.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @return: A success status will be returned if protocol was successfully added.
 *  PPP_NO_SPACE will be returned if there is not enough space on the buffer to
 *  add protocol.
 * This function will add PPP protocol field in the given buffer.
 */
int32_t ppp_packet_protocol_add(FS_BUFFER *buffer, uint16_t protocol, uint8_t pfc, uint8_t flags)
{
    int32_t status;
    uint8_t proto_len = 2;

    /* If we can compress the protocol field. */
    if (((protocol & 0xFF00) == 0) && (pfc == TRUE))
    {
        /* We will only add the non-zero byte of the protocol field. */
        proto_len = 1;
    }

    /* Push protocol for this packet. */
    status = fs_buffer_push(buffer, &(protocol), proto_len, (FS_BUFFER_PACKED | FS_BUFFER_HEAD | flags));

    /* Return status to the caller. */
    return (status);

} /* ppp_packet_protocol_add */

/*
 * ppp_packet_configuration_header_parse
 * @buffer: Buffer from which PPP configuration header is needed to be parsed.
 * @packet: PPP packet structure which will be populated with the parsed data.
 * @return: A success status will be returned if header was successfully parsed,
 *  PPP_INVALID_HEADER will be returned if an invalid header was parsed.
 * This function will parse the PPP header for configuration packets in the
 * given buffer and populate the PPP packet structure.
 */
int32_t ppp_packet_configuration_header_parse(FS_BUFFER *buffer, PPP_CONF_PKT *packet)
{
    int32_t status = SUCCESS;

    /* We must have 4 bytes on the buffer to pull the PPP configuration header. */
    if (buffer->total_length >= 4)
    {
        /* Pull the code, id and length. */
        OS_ASSERT(fs_buffer_pull(buffer, &packet->code, 1, 0) != SUCCESS);
        OS_ASSERT(fs_buffer_pull(buffer, &packet->id, 1, 0) != SUCCESS);
        OS_ASSERT(fs_buffer_pull(buffer, &packet->length, 2, FS_BUFFER_PACKED) != SUCCESS);

        /* If header has invalid length of data left. */
        if (buffer->total_length != (uint32_t)(packet->length - 4))
        {
            /* This is a parsing error. */
            status = PPP_INVALID_HEADER;
        }
    }
    else
    {
        /* This is a parsing error. */
        status = PPP_INVALID_HEADER;
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_packet_configuration_header_parse */

/*
 * ppp_packet_configuration_option_parse
 * @buffer: Buffer from which a configuration option is needed to be parsed.
 * @option: PPP option structure that will be populated with the parsed values.
 * @return: A success status will be returned if option was successfully parsed,
 *  PPP_NO_NEXT_OPTION will be returned if there is no option to parse,
 *  PPP_INVALID_HEADER will be returned if an invalid header was parsed.
 * This function will parse a PPP option in the given buffer and populate the
 * PPP option structure.
 */
int32_t ppp_packet_configuration_option_parse(FS_BUFFER *buffer, PPP_CONF_OPT *option)
{
    int32_t status = SUCCESS;

    /* If we have enough data on buffer to pull the option. */
    if (buffer->total_length >= 2)
    {
        /* Pull the option type and length. */
        OS_ASSERT(fs_buffer_pull(buffer, &option->type, 1, 0) != SUCCESS);
        OS_ASSERT(fs_buffer_pull(buffer, &option->length, 1, 0) != SUCCESS);

        /* If this option has some data. */
        if (option->length > 2)
        {
            /* Validate that buffer has enough length left to process this
             * option. */
            if ((buffer->total_length >= (uint32_t)(option->length - 2)) &&
                ((uint32_t)(option->length - 2) <= PPP_MAX_OPTION_SIZE))
            {
                /* Just pull the data and copy it in the option data buffer. */
                OS_ASSERT(fs_buffer_pull(buffer, option->data, (uint32_t)(option->length - 2), 0) != SUCCESS);
            }
            else
            {
                /* Should not happen return an error. */
                status = PPP_INVALID_HEADER;
            }
        }
    }

    /* If we don't have any data left on the buffer to parse. */
    else if (buffer->total_length == 0)
    {
        /* There is no next option to process. */
        status = PPP_NO_NEXT_OPTION;
    }

    /* There is still some data but not enough to make an option out of it. */
    else
    {
        /* Should not happen return an error. */
        status = PPP_INVALID_HEADER;
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_packet_configuration_option_parse */

/*
 * ppp_packet_configuration_header_add
 * @buffer: Buffer on which configuration header will be added.
 * @packet: PPP Packet header needed to be added.
 * @return: A success status will be returned if header was successfully added,
 *  PPP_NO_SPACE will be returned if there is no space on the buffer to add the
 *  header.
 * This function will add a PPP configuration option header in the provided buffer.
 */
int32_t ppp_packet_configuration_header_add(FS_BUFFER *buffer, PPP_CONF_PKT *packet)
{
    int32_t status = SUCCESS;

    /* Calculate the packet length. */
    packet->length = (uint16_t)(buffer->total_length + 4);

    /* Push length for this packet. */
    status = fs_buffer_push(buffer, &packet->length, 2, (FS_BUFFER_PACKED | FS_BUFFER_HEAD));

    /* If length was successfully added. */
    if (status == SUCCESS)
    {
        /* Push configuration packet ID. */
        status = fs_buffer_push(buffer, &packet->id, 1, (FS_BUFFER_HEAD));
    }

    /* If packet ID was successfully added. */
    if (status == SUCCESS)
    {
        /* Push configuration packet code. */
        status = fs_buffer_push(buffer, &packet->code, 1, (FS_BUFFER_HEAD));
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_packet_configuration_header_add */

/*
 * ppp_packet_configuration_option_add
 * @buffer: Buffer in which this option will be added.
 * @option: Option needed to be added.
 * @return: A success status will be returned if option was successfully added,
 *  PPP_NO_SPACE will be returned if there is no space on the buffer to add the
 *  data.
 * This function will add a PPP configuration option in the provided buffer.
 */
int32_t ppp_packet_configuration_option_add(FS_BUFFER *buffer, PPP_CONF_OPT *option)
{
    int32_t status = SUCCESS;

    /* Add type of this option. */
    status = fs_buffer_push(buffer, &option->type, 1, 0);

    /* If type was successfully added. */
    if (status == SUCCESS)
    {
        /* Add length for this option. */
        status = fs_buffer_push(buffer, &option->length, 1, 0);
    }

    /* Check if we need to add value data in the option. */
    if (option->length > 2)
    {
        /* Add the option value. */
        status = fs_buffer_push(buffer, option->data, (uint32_t)(option->length - 2), 0);
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_packet_configuration_option_add */

#endif /* CONFIG_PPP */
