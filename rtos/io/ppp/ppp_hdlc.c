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
#include <string.h>

/*
 * hdlc_parse_header
 * @buffer: Buffer needed to be processed.
 * @acfc: If address and control fields may be compressed.
 * @return: A success status will be returned if header was successfully parsed,
 *  PPP_INVALID_HEADER will be returned if an invalid header was parsed.
 * This function will process the HDLC header. Nothing is needed to be returned
 * here other then verifying and stripping the FCS at the end of the packet and
 * verifying other constant data like flags, address and control fields.
 */
int32_t ppp_hdlc_header_parse(FS_BUFFER_CHAIN *buffer, uint8_t acfc)
{
    int32_t status = SUCCESS;
    uint8_t flag = 0;
    uint8_t acf[2];

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
        if (buffer->total_length >= 6)
        {
            /* Peek the start buffer. */
            OS_ASSERT(fs_buffer_chain_pull(buffer, (char *)&flag, 1, FS_BUFFER_INPLACE) != SUCCESS);

            /* Validate the start buffer. */
            if (flag != PPP_FLAG)
            {
                /* Return an error. */
                status = PPP_INVALID_HEADER;
            }
        }
        else
        {
            /* Return an error. */
            status = PPP_INVALID_HEADER;
        }
    }

    if (status == SUCCESS)
    {
        /* Clear the previous flag. */
        flag = 0;

        /* Peek the last byte. */
        OS_ASSERT(fs_buffer_chain_pull(buffer, (char *)&flag, 1, (FS_BUFFER_INPLACE | FS_BUFFER_TAIL)) != SUCCESS);

        /* Validate the end buffer. */
        if (flag != PPP_FLAG)
        {
            /* Return an error. */
            status = PPP_INVALID_HEADER;
        }
    }

    if (status == SUCCESS)
    {
        /* Skim the start flag. */
        OS_ASSERT(fs_buffer_chain_pull(buffer, NULL, 1, 0) != SUCCESS);

        /* Pull the end flag. */
        OS_ASSERT(fs_buffer_chain_pull(buffer, NULL, 1, FS_BUFFER_TAIL) != SUCCESS);

        /* Compute and verify the FCS. */
        if (PPP_FCS16_IS_VALID(buffer->list.head))
        {
            /* Pull the FCS from the buffer.
             * RFC-1662: The FCS field is calculated over all bits of the
             * Address, Control, Protocol, Information and Padding fields,
             * not including any start and stop bits (asynchronous) nor any
             * bits (synchronous) or octets (asynchronous or synchronous)
             * inserted for transparency.  This also does not include the Flag
             * Sequences nor the FCS field itself.*/
            OS_ASSERT(fs_buffer_chain_pull(buffer, NULL, 2, FS_BUFFER_TAIL) != SUCCESS);

            /* Peek the address and control fields. */
            OS_ASSERT(fs_buffer_chain_pull(buffer, (char *)acf, 2, FS_BUFFER_INPLACE) != SUCCESS);

            /* Verify that we have address and control fields as specified
             * by the RFC-1662. */
            if ((acf[0] == PPP_ADDRESS) && (acf[1] == PPP_CONTROL))
            {
                /* Skim the address and control fields. */
                OS_ASSERT(fs_buffer_chain_pull(buffer, NULL, 2, 0) != SUCCESS);

                /* HDLC frame was successfully verified. */
                status = SUCCESS;
            }

            /* When ACFC is negotiated these two fields will be left out.  */
            else if ( ((acf[0] != PPP_ADDRESS) || (acf[1] != PPP_CONTROL)) &&
                      (acfc == TRUE) )
            {
                /* Address and control field are elided. */
                /* HDLC frame was successfully verified. */
                status = SUCCESS;
            }
        }
    }
    else
    {
        status = -1;
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_header_parse */

/*
 * ppp_hdlc_unescape
 * @buffer: Buffer chain needed to be processed.
 * @return: A success status will be returned if given buffer was successfully
 *  escaped. HDLC_STREAM_ERROR will be returned in a stream error was detected.
 * This function will un-escaping a HDLC packet.
 */
int32_t ppp_hdlc_unescape(FS_BUFFER_CHAIN *buffer)
{
    FS_BUFFER *this_buffer = buffer->list.head;
    int32_t status = SUCCESS;
    uint8_t last_escaped = FALSE;

    /* Reset the buffer chain length. */
    buffer->total_length = 0;

    /* While we have a buffer in the chain. */
    while (this_buffer != NULL)
    {
        /* Un-escape this buffer. */
        ppp_hdlc_unescape_one(this_buffer, &last_escaped);

        /* Add the length for converted buffer. */
        buffer->total_length += this_buffer->length;

        /* Pick the next buffer in the list. */
        this_buffer = this_buffer->next;
    }

    if (last_escaped == TRUE)
    {
        /* Should not happen return an error. */
        status = HDLC_STREAM_ERROR;
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_unescape */

/*
 * ppp_hdlc_unescape_one
 * @buffer: Buffer needed to be processed.
 * This function will un-escaping a HDLC buffer.
 */
void ppp_hdlc_unescape_one(FS_BUFFER *buffer, uint8_t *last_escaped)
{
    char *data = buffer->buffer;
    uint32_t converted = 0;

    /* If we need to escape first byte on this buffer. */
    if ((buffer->length > 0) && (*last_escaped == TRUE))
    {
        /* Compute the byte to return. */
        data[converted] = (buffer->buffer[0] ^ 0x20);
        converted ++;
        *last_escaped = FALSE;

        /* Consume this byte. */
        OS_ASSERT(fs_buffer_one_pull(buffer, NULL, 1, 0) != SUCCESS);
    }

    /* While we have data in the source buffer. */
    while (buffer->length > 0)
    {
        /* If we have a escape sequence. */
        if (buffer->buffer[0] == PPP_ESCAPE)
        {
            /* If we have enough length on source buffer. */
            if (buffer->length > 1)
            {
                /* Compute the byte to return. */
                data[converted] = (buffer->buffer[1] ^ 0x20);
                converted ++;

                /* Consume 2 bytes. */
                OS_ASSERT(fs_buffer_one_pull(buffer, NULL, 2, 0) != SUCCESS);
            }
            else
            {
                /* Consume this byte. */
                OS_ASSERT(fs_buffer_one_pull(buffer, NULL, 1, 0) != SUCCESS);

                /* First byte in the next buffer is needed to be escaped. */
                *last_escaped = TRUE;
            }
        }

        else
        {
            /* Put this byte on the destination buffer. */
            data[converted] = buffer->buffer[0];
            converted ++;

            /* Consume this byte. */
            OS_ASSERT(fs_buffer_one_pull(buffer, NULL, 1, 0) != SUCCESS);
        }
    }

    /* if we have processed the whole stream. */
    if (buffer->length == 0)
    {
        /* Reinitialize buffer data. */
        fs_buffer_update(buffer, data, converted);
    }

} /* ppp_hdlc_unescape_one */

/*
 * ppp_hdlc_header_add
 * @buffer: Buffer needed to be processed.
 * @accm: Array of 4 bytes of transmit ACCM to be used to escape the data.
 * @acfc: If address and control fields may be compressed.
 * @lcp: If we are sending a LCP request.
 * @return: A success status will be returned if header was successfully added,
 *  PPP_NO_SPACE will be returned if there was not enough space on the buffer
 *  to add this header.
 * This function will process the HDLC header. Nothing is needed to be returned
 * here other then verifying and stripping the FCS at the end of the packet and
 * verifying other constant data like flags, address and control fields.
 * [TDOD] Add support for multiple buffers here or multiple packets in a buffer.
 */
int32_t ppp_hdlc_header_add(FS_BUFFER_CHAIN *buffer, uint32_t *accm, uint8_t acfc, uint8_t lcp)
{
    int32_t status = SUCCESS;
    uint8_t value_uint8;
    uint16_t fcs;
    FS_BUFFER_CHAIN destination;

    /* When ACFC is negotiated we can optionally drop address and control
     * fields. */
    if ((lcp == TRUE) || (acfc == FALSE))
    {
        /* Add control field. */
        value_uint8 = PPP_CONTROL;
        OS_ASSERT(fs_buffer_chain_push(buffer, (char *)&value_uint8, 1, FS_BUFFER_HEAD) != SUCCESS);

        /* Add address field. */
        value_uint8 = PPP_ADDRESS;
        OS_ASSERT(fs_buffer_chain_push(buffer, (char *)&value_uint8, 1, FS_BUFFER_HEAD) != SUCCESS);
    }

    /* Calculate the FCS of the data. */
    fcs = ppp_fcs16_buffer_calculate(buffer->list.head, PPP_FCS16_INIT);
    fcs ^= 0xffff;

    /* Push the FCS at the end of buffer. */
    OS_ASSERT(fs_buffer_chain_push(buffer, (char *)&fcs, 2, 0) != SUCCESS);

    /* Initialize a destination chain buffer. */
    memset(&destination, 0, sizeof(FS_BUFFER_CHAIN));
    destination.fd = buffer->fd;

    /* Escape the data. */
    status = ppp_hdlc_escape(buffer, &destination, accm, lcp);

    /* If there is still some data left on the source buffer then the status
     * is not success, we still need to clean up the things. */
    if (buffer->total_length > 0)
    {
        /* Free the remaining buffers. */
        fs_buffer_chain_add(buffer, FS_BUFFER_TX, FS_BUFFER_ACTIVE);
    }

    /* Copy our chain buffer to the provided buffer. */
    memcpy(buffer, &destination, sizeof(FS_BUFFER_CHAIN));

    /* If data was successfully escaped. */
    if (status == SUCCESS)
    {
        /* Add start and end flags. */
        value_uint8 = PPP_FLAG;
        OS_ASSERT(fs_buffer_chain_push(buffer, (char *)&value_uint8, 1, FS_BUFFER_HEAD) != SUCCESS);
        OS_ASSERT(fs_buffer_chain_push(buffer, (char *)&value_uint8, 1, 0) != SUCCESS);
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_header_add */

/*
 * ppp_hdlc_escape
 * @src: Buffer needed to be processed.
 * @dst: Buffer in which escaped packet will be pushed.
 * @accm: Array of 4 bytes of transmit ACCM.
 * @lcp: If we are sending a LCP request.
 * This function will escape a HDLC buffer. This routine will leave room for
 * end and start flags.
 */
int32_t ppp_hdlc_escape(FS_BUFFER_CHAIN *src, FS_BUFFER_CHAIN *dst, uint32_t *accm, uint8_t lcp)
{
    int32_t status = SUCCESS;
    uint8_t buf[2];

    /* While we have some data left in source buffer. */
    while ((status == SUCCESS) && (src->total_length > 0))
    {
        /* Pull a byte from the source buffer chain. */
        OS_ASSERT(fs_buffer_chain_pull(src, (char *)buf, 1, 0) != SUCCESS);

        /* Check if we need to escape this byte. */
        if ( ((lcp == TRUE) && (*buf < 0x20)) ||
              (accm[*buf >> 5] & (uint32_t)(1 << (*buf & 0x1F))) )
        {
            /* Escape this character. */
            buf[1] = buf[0] ^ 0x20;
            buf[0] = PPP_ESCAPE;

            /* Push converted byte on the destination. */
            status = fs_buffer_chain_push(dst, (char *)buf, 2, 0);
        }
        else
        {
            /* Push the byte as it is. */
            status = fs_buffer_chain_push(dst, (char *)buf, 1, 0);
        }
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_escape */

#endif /* CONFIG_PPP */
