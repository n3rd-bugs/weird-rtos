/*
 * ppp_hdlc.c
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

/*
 * hdlc_parse_header
 * @buffer: Buffer needed to be processed.
 * @acfc: If address and control fields may be compressed.
 * @return: A success status will be returned if header was successfully parsed,
 *  PPP_INVALID_HEADER will be returned if an invalid header was parsed.
 * This function will process the HDLC header. Nothing is needed to be returned
 * here other then verifying and stripping the FCS at the end of the packet and
 * verifying other constant data like flags, address and control fields.
 * [TDOD] Add support for multiple buffers here or multiple packets in a buffer.
 */
int32_t ppp_hdlc_header_parse(FS_BUFFER *buffer, uint8_t acfc)
{
    int32_t status;

    /* First un-escape the data. */
    status = ppp_hdlc_unescape(buffer);

    if (status == SUCCESS)
    {
        /* RFC-1662:
         * +----------+----------+----------+----------+----------+----------+----------+
         * |   Flag   | Address  | Control  | Protocol |    PPP   |   FCS    |   Flag   |
         * | 01111110 | 11111111 | 00000011 | 8/16 bits|  Payload |16/32 bits| 01111110 |
         * +----------+----------+----------+----------+----------+----------+----------+
         */

        /* To successfully parse HDLC frame we must need 6 bytes on the buffer. */
        if ( (buffer->length >= 6) &&

             /* Verify that we have valid flag. */
             ((uint8_t)buffer->buffer[0] == PPP_FLAG) &&

             /* Check if we have a valid flag at the end of data. */
             ((uint8_t)buffer->buffer[buffer->length - 1] == PPP_FLAG) )
        {
            /* Skim the start flag. */
            OS_ASSERT(fs_buffer_pull(buffer, NULL, 1, 0) != SUCCESS);

            /* Pull the end flag. */
            OS_ASSERT(fs_buffer_pull(buffer, NULL, 1, FS_BUFFER_TAIL) != SUCCESS);

            /* Compute and verify the FCS. */
            if (PPP_FCS16_IS_VALID(buffer->buffer, buffer->length))
            {
                /* Pull the FCS from the buffer.
                 * RFC-1662: The FCS field is calculated over all bits of the
                 * Address, Control, Protocol, Information and Padding fields,
                 * not including any start and stop bits (asynchronous) nor any
                 * bits (synchronous) or octets (asynchronous or synchronous)
                 * inserted for transparency.  This also does not include the Flag
                 * Sequences nor the FCS field itself.*/
                OS_ASSERT(fs_buffer_pull(buffer, NULL, 2, FS_BUFFER_TAIL) != SUCCESS);

                /* Verify that we have address and control fields as specified
                 * by the RFC-1662. */
                if ( ((uint8_t)buffer->buffer[0] == PPP_ADDRESS) &&
                     ((uint8_t)buffer->buffer[1] == PPP_CONTROL) )
                {
                    /* Skim the address and control fields. */
                    OS_ASSERT(fs_buffer_pull(buffer, NULL, 2, 0) != SUCCESS);

                    /* HDLC frame was successfully verified. */
                    status = SUCCESS;
                }

                /* When ACFC is negotiated these two fields will be left out.  */
                else if ( ( ((uint8_t)buffer->buffer[0] != PPP_ADDRESS) ||
                            ((uint8_t)buffer->buffer[1] != PPP_CONTROL) ) &&
                          (acfc == TRUE) )
                {
                    /* Address and control field are elided. */
                    /* HDLC frame was successfully verified. */
                    status = SUCCESS;
                }
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_header_parse */

/*
 * ppp_hdlc_header_add
 * @buffer: Buffer needed to be processed.
 * @accm: Array of 4 bytes of transmit ACCM to be used to escape the data.
 * @acfc: If address and control fields may be compressed.
 * @return: A success status will be returned if header was successfully added,
 *  PPP_NO_SPACE will be returned if there was not enough space on the buffer
 *  to add this header.
 * This function will process the HDLC header. Nothing is needed to be returned
 * here other then verifying and stripping the FCS at the end of the packet and
 * verifying other constant data like flags, address and control fields.
 * [TDOD] Add support for multiple buffers here or multiple packets in a buffer.
 */
int32_t ppp_hdlc_header_add(FS_BUFFER *buffer, uint32_t *accm, uint8_t acfc)
{
    int32_t status = SUCCESS;
    uint8_t value_uint8;
    uint16_t fcs;

    /* If we have enough space on the buffer. */
    if (((buffer->max_length - buffer->length) - (uint32_t)(buffer->buffer - buffer->data)) > 6)
    {
        /* When ACFC is negotiated we can optionally drop address and control
         * fields. */
        if (acfc == FALSE)
        {
            /* Add control field. */
            value_uint8 = PPP_CONTROL;
            OS_ASSERT(fs_buffer_push(buffer, (char *)&value_uint8, 1, FS_BUFFER_HEAD) != SUCCESS);

            /* Add address field. */
            value_uint8 = PPP_ADDRESS;
            OS_ASSERT(fs_buffer_push(buffer, (char *)&value_uint8, 1, FS_BUFFER_HEAD) != SUCCESS);
        }

        /* Calculate the FCS of the data. */
        fcs = ppp_fcs16_calculate(buffer->buffer, buffer->length);
        fcs ^= 0xffff;

        /* Push the FCS at the end of buffer. */
        OS_ASSERT(fs_buffer_push(buffer, (char *)&fcs, 2, 0) != SUCCESS);

        /* Escape the data. */
        status = ppp_hdlc_escape(buffer, accm);

        /* If data was successfully escaped. */
        if (status == SUCCESS)
        {
            /* Add start and end flags. */
            value_uint8 = PPP_FLAG;
            OS_ASSERT(fs_buffer_push(buffer, (char *)&value_uint8, 1, FS_BUFFER_HEAD) != SUCCESS);
            OS_ASSERT(fs_buffer_push(buffer, (char *)&value_uint8, 1, 0) != SUCCESS);
        }
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_header_add */

/*
 * ppp_hdlc_escape
 * @buffer: Buffer needed to be processed.
 * @accm: Array of 4 bytes of transmit ACCM.
 * This function will escape a HDLC buffer. This routine will leave room for
 * end and start flags.
 */
int32_t ppp_hdlc_escape(FS_BUFFER *buffer, uint32_t *accm)
{
    int32_t status = PPP_NO_SPACE;
    uint8_t buf[2];
    uint8_t *process_ptr = (uint8_t *)&buffer->buffer[buffer->length - 1];
    uint32_t length = buffer->length;

    /* Adjust the buffer in such a way that we can add the data at the end of
     * it. */
    buffer->length = 0;
    buffer->buffer = (&buffer->data[buffer->max_length - 1] - 1);   /* Leave room for end flag. */

    /* While we have some data to process. */
    while ((length > 0) && (process_ptr < (uint8_t *)(buffer->buffer - 2)))
    {
        /* Check if we need to escape this byte. */
        if (accm[*process_ptr >> 5] & (uint32_t)(1 << (*process_ptr & 0x1F)))
        {
            /* Escape this character. */
            buf[0] = PPP_ESCAPE;
            buf[1] = (uint8_t)(*process_ptr ^ 0x20);

            /* Push this on the buffer. */
            OS_ASSERT(fs_buffer_push(buffer, (char *)buf, 2, FS_BUFFER_HEAD) != SUCCESS);
        }
        else
        {
            /* Put this character as it is. */
            OS_ASSERT(fs_buffer_push(buffer, (char *)process_ptr, 1, FS_BUFFER_HEAD) != SUCCESS);
        }

        /* We have processed this byte. */
        process_ptr--;
        length--;
    }

    /* All the data was successfully escaped. */
    if (length == 0)
    {
        /* Return success. */
        status = SUCCESS;
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_escape */

/*
 * ppp_hdlc_unescape
 * @buffer: Buffer needed to be processed.
 * This function will un-escaping a HDLC packet.
 */
int32_t ppp_hdlc_unescape(FS_BUFFER *buffer)
{
    int32_t status = SUCCESS;
    char *data = buffer->buffer;
    uint32_t converted = 0;

    /* While we have data in the source buffer. */
    while ((status == SUCCESS) && (buffer->length > 0))
    {
        /* If we have a escape sequence. */
        if (buffer->buffer[0] == PPP_ESCAPE)
        {
            /* If we have enough length on source buffer. */
            if (buffer->length > 1)
            {
                /* Compute the byte to return. */
                data[converted] = (buffer->buffer[1] ^ 0x20);

                /* Consume 2 bytes. */
                OS_ASSERT(fs_buffer_pull(buffer, NULL, 2, 0) != SUCCESS);
            }
            else
            {
                /* Stream error. */
                status = HDLC_STREAM_ERROR;
            }
        }

        else
        {
            /* Put this byte on the destination buffer. */
            data[converted] = buffer->buffer[0];

            /* Consume this byte. */
            OS_ASSERT(fs_buffer_pull(buffer, NULL, 1, 0) != SUCCESS);
        }

        if (status == SUCCESS)
        {
            /* In each go we will be pushing one byte on the buffer. */
            converted ++;

            /* if we have processed the whole stream. */
            if (buffer->length == 0)
            {
                /* Reinitialize buffer data. */
                fs_buffer_update(buffer, data, converted);

                /* Break out of this loop. */
                break;
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_unescape */

#endif /* CONFIG_PPP */
