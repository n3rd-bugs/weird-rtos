/*
 * ppp_hdlc.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef IO_PPP
#include <string.h>
#include <fs.h>
#include <ppp.h>
#include <ppp_hdlc.h>

/*
 * ppp_hdlc_header_parse
 * @buffer: Buffer chain needed to be processed.
 * @acfc: If address and control fields may be compressed.
 * @return: A success status will be returned if header was successfully parsed,
 *  PPP_INVALID_HEADER will be returned if an invalid header was parsed.
 * This function will process the HDLC header. Nothing is needed to be returned
 * here other then verifying and stripping the FCS at the end of the packet and
 * verifying other constant data like flags, address and control fields.
 */
int32_t ppp_hdlc_header_parse(FS_BUFFER_LIST *buffer, uint8_t acfc)
{
    int32_t status = SUCCESS;
    uint8_t flag = 0;
    uint8_t acf[2];

    /* First un-escape the data. */
    status = ppp_hdlc_unescape(buffer);

    /* RFC-1662:
     * +----------+----------+----------+----------+----------+----------+----------+
     * |   Flag   | Address  | Control  | Protocol |    PPP   |   FCS    |   Flag   |
     * | 01111110 | 11111111 | 00000011 | 8/16 bits|  Payload |16/32 bits| 01111110 |
     * +----------+----------+----------+----------+----------+----------+----------+
     * We will process start/end flags, address and control fields and the
     * trailing FCS at the end.
     */

    /* Verify start flag. */
    if (status == SUCCESS)
    {
        /* To successfully parse HDLC frame we must need 4 bytes on the buffer. */
        if (buffer->total_length >= 4)
        {
            /* Peek the start buffer. */
            ASSERT(fs_buffer_list_pull(buffer, &flag, 1, FS_BUFFER_INPLACE) != SUCCESS);

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

    /* Verify end flag. */
    if (status == SUCCESS)
    {
        /* Clear the previous flag. */
        flag = 0;

        /* Peek the last byte. */
        ASSERT(fs_buffer_list_pull(buffer, &flag, 1, (FS_BUFFER_INPLACE | FS_BUFFER_TAIL)) != SUCCESS);

        /* Validate the end buffer. */
        if (flag != PPP_FLAG)
        {
            /* Return an error. */
            status = PPP_INVALID_HEADER;
        }
    }

    /* If start and end flags were successfully parsed. */
    if (status == SUCCESS)
    {
        /* Skim the start and end flags. */
        ASSERT(fs_buffer_list_pull(buffer, NULL, 1, 0) != SUCCESS);
        ASSERT(fs_buffer_list_pull(buffer, NULL, 1, FS_BUFFER_TAIL) != SUCCESS);

        /* Compute and verify the FCS. */
        if (PPP_FCS16_IS_VALID(buffer))
        {
            /* Pull the FCS from the buffer.
             * RFC-1662: The FCS field is calculated over all bits of the
             * Address, Control, Protocol, Information and Padding fields,
             * not including any start and stop bits (asynchronous) nor any
             * bits (synchronous) or octets (asynchronous or synchronous)
             * inserted for transparency.  This also does not include the Flag
             * Sequences nor the FCS field itself.*/
            ASSERT(fs_buffer_list_pull(buffer, NULL, 2, FS_BUFFER_TAIL) != SUCCESS);

            if (buffer->total_length >= 2)
            {
                /* Peek the address and control fields. */
                ASSERT(fs_buffer_list_pull(buffer, acf, 2, FS_BUFFER_INPLACE) != SUCCESS);

                /* Verify that we have address and control fields as specified
                 * by the RFC-1662. */
                if ((acf[0] == PPP_ADDRESS) && (acf[1] == PPP_CONTROL))
                {
                    /* Skim the address and control fields. */
                    ASSERT(fs_buffer_list_pull(buffer, NULL, 2, 0) != SUCCESS);
                }

                /* When ACFC is negotiated these two fields can be left out, if
                 * not these should have been inline.  */
                else if ( ((acf[0] != PPP_ADDRESS) || (acf[1] != PPP_CONTROL)) &&
                          (acfc != TRUE) )
                {
                    /* Return an error here. */
                    status = PPP_INVALID_HEADER;
                }
            }

            /* If ACFC is not enabled we should have something here. */
            else if (acfc != TRUE)
            {
                /* Return an error here. */
                status = PPP_INVALID_HEADER;
            }
        }
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
int32_t ppp_hdlc_unescape(FS_BUFFER_LIST *buffer)
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

    /* If we have got a escape flag but no next byte. */
    if (last_escaped == TRUE)
    {
        /* This is a parsing error, return an error. */
        status = HDLC_STREAM_ERROR;
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_unescape */

/*
 * ppp_hdlc_unescape_one
 * @buffer: Buffer needed to be processed.
 * @last_escaped: If we got a escape byte at the end of a buffer, this flag
 *  will be set to true so that we can escape the first byte on next buffer.
 * This function will un-escaping a HDLC buffer.
 */
void ppp_hdlc_unescape_one(FS_BUFFER *buffer, uint8_t *last_escaped)
{
    uint8_t *data = buffer->buffer;
    uint32_t converted = 0;

    /* If we need to escape first byte on this buffer. */
    if ((buffer->length > 0) && (*last_escaped == TRUE))
    {
        /* Escape the first byte. */
        data[converted] = (buffer->buffer[0] ^ 0x20);
        converted ++;

        /* Reset the escape flag. */
        *last_escaped = FALSE;

        /* Consume this byte. */
        ASSERT(fs_buffer_pull(buffer, NULL, 1, 0) != SUCCESS);
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
                /* Escape this byte and put it back on the buffer in-line with
                 * other bytes. */
                data[converted] = (buffer->buffer[1] ^ 0x20);
                converted ++;

                /* Consume 2 bytes from the given buffer. */
                ASSERT(fs_buffer_pull(buffer, NULL, 2, 0) != SUCCESS);
            }
            else
            {
                /* Consume this byte from the given buffer. */
                ASSERT(fs_buffer_pull(buffer, NULL, 1, 0) != SUCCESS);

                /* First byte in the next buffer is needed to be escaped. */
                *last_escaped = TRUE;
            }
        }

        else
        {
            /* Put this byte on the destination buffer as it is. */
            data[converted] = buffer->buffer[0];
            converted ++;

            /* Consume this byte from the given buffer. */
            ASSERT(fs_buffer_pull(buffer, NULL, 1, 0) != SUCCESS);
        }
    }

    /* If we have processed the whole buffer. */
    if (buffer->length == 0)
    {
        /* Reinitialize buffer data. */
        fs_buffer_update(buffer, data, converted);
    }

} /* ppp_hdlc_unescape_one */

/*
 * ppp_hdlc_header_add
 * @buffer: Buffer on which we need to put an HDLC header.
 * @accm: Array of 4 bytes of transmit ACCM to be used to escape the data.
 * @acfc: If address and control fields may be compressed.
 * @lcp: If we are sending a LCP request.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @return: A success status will be returned if header was successfully added,
 *  PPP_NO_SPACE will be returned if there was not enough space on the buffer
 *  to add this header, PPP_NO_BUFFERS will be returned if we don't have a free
 *  buffer to process this PPP packet.
 * This function will add an HDLC header on the given buffer, also this function
 * is responsible for escaping the data after computing and appending the FCS.
 */
int32_t ppp_hdlc_header_add(FS_BUFFER_LIST *buffer, uint32_t *accm, uint8_t acfc, uint8_t lcp, uint8_t flags)
{
    FS_BUFFER_LIST *destination = fs_buffer_get(buffer->fd, FS_LIST_FREE, 0);
    int32_t status = SUCCESS;
    uint16_t fcs;

    /* If we do have a destination buffer. */
    if (destination)
    {
        /* When ACFC is negotiated we can optionally drop address and control
         * fields. We will still need to add these fields if we are sending a LCP
         * frame. */
        if ((lcp == TRUE) || (acfc == FALSE))
        {
            /* Add control field. */
            status = fs_buffer_list_push(buffer, (uint8_t []){ (uint8_t)PPP_CONTROL }, 1, (FS_BUFFER_HEAD | flags));

            if (status == SUCCESS)
            {
                /* Add address field. */
                status = fs_buffer_list_push(buffer, (uint8_t []){ (uint8_t)PPP_ADDRESS }, 1, (FS_BUFFER_HEAD | flags));
            }
        }

        /* If address and control fields were successfully added. */
        if (status == SUCCESS)
        {
            /* Calculate the FCS of the data. */
            fcs = ppp_fcs16_buffer_calculate(buffer, PPP_FCS16_INIT);
            fcs ^= 0xffff;

            /* Push the FCS at the end of buffer. */
            status = fs_buffer_list_push(buffer, &fcs, 2, flags);
        }

        /* If FCS was successfully appended. */
        if (status == SUCCESS)
        {
            /* Escape the given buffer and initialize an other buffer with the result. */
            status = ppp_hdlc_escape(buffer, destination, accm, lcp, flags);
        }

        /* If data was successfully escaped. */
        if (status == SUCCESS)
        {
            /* Save the list member. */
            destination->next = buffer->next;

            /* Move the generated buffer back to the original buffer. */
            fs_buffer_list_move(buffer, destination);

            /* Add start flag. */
            status = fs_buffer_list_push(buffer, (uint8_t []){ PPP_FLAG }, 1, (FS_BUFFER_HEAD | flags));

            /* If start flag was successfully added. */
            if (status == SUCCESS)
            {
                /* Add end flag. */
                status = fs_buffer_list_push(buffer, (uint8_t []){ PPP_FLAG }, 1, flags);
            }
        }

        /* Free the destination buffer as it is no longer required. */
        fs_buffer_add(buffer->fd, destination, FS_LIST_FREE, FS_BUFFER_ACTIVE);
    }
    else
    {
        /* No more buffers available to process this PPP packet. */
        status = PPP_NO_BUFFERS;
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_header_add */

/*
 * ppp_hdlc_escape
 * @src: Buffer needed to be processed.
 * @dst: Buffer in which escaped packet will be pushed.
 * @accm: Array of 4 bytes of the negotiated transmit ACCM.
 * @lcp: True if we are sending a LCP request.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * This function will escape a HDLC buffer. The result will be larger than the
 * provided buffer so we cannot push the data in the same buffer.
 */
int32_t ppp_hdlc_escape(FS_BUFFER_LIST *src, FS_BUFFER_LIST *dst, uint32_t *accm, uint8_t lcp, uint8_t flags)
{
    int32_t status = SUCCESS;
    uint8_t buf[2];

    /* While we have some data left in source buffer. */
    while ((status == SUCCESS) && (src->total_length > 0))
    {
        /* Pull a byte from the source buffer chain. */
        ASSERT(fs_buffer_list_pull(src, buf, 1, 0) != SUCCESS);

        /* Check if we need to escape this byte. */
        if ( ((lcp == TRUE) && (*buf < 0x20)) ||
              (accm[*buf >> 5] & (uint32_t)(1 << (*buf & 0x1F))) )
        {
            /* Escape this character. */
            buf[1] = buf[0] ^ 0x20;
            buf[0] = PPP_ESCAPE;

            /* Push converted byte on the destination. */
            status = fs_buffer_list_push(dst, buf, 2, flags);
        }
        else
        {
            /* Push the byte as it is. */
            status = fs_buffer_list_push(dst, buf, 1, flags);
        }
    }

    /* Return status to the caller. */
    return (status);

} /* ppp_hdlc_escape */

#endif /* IO_PPP */
