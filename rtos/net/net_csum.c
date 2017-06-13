/*
 * net_csum.c
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
#include <net_csum.h>
#include <header.h>

/*
 * net_pseudo_csum_calculate
 * @buffer: File buffer for which checksum is required.
 * @src_ip: Source IP address.
 * @dst_ip: Destination IP address.
 * @protocol: Protocol for which pseudo checksum is needed to be calculated.
 * @offset: Offset in the buffer at which the actual protocol header lies.
 * @flags: Operation flags.
 *  FS_BUFFER_TH: We need to maintain threshold while allocating a buffer.
 * @csum: Pointer where checksum will be returned.
 * @return: A success status will be returned if TCP checksum was successfully
 *  calculated.
 *  FS_BUFFER_NO_SPACE will be returned if we don't have any buffers to
 *  calculate the checksum.
 * This function will calculate pseudo checksum for the given packet and
 * protocol.
 */
int32_t net_pseudo_csum_calculate(FS_BUFFER *buffer, uint32_t src_ip, uint32_t dst_ip, uint8_t protocol, uint16_t length, uint32_t offset, uint8_t flags, uint16_t *csum)
{
    int32_t status;
    uint32_t ret_csum;
    FS_BUFFER *csum_buffer;
    HDR_GEN_MACHINE hdr_machine;
    HEADER pseudo_hdr[] =
    {
        {&src_ip,           4, (FS_BUFFER_PACKED | flags) },    /* Source address. */
        {&dst_ip,           4, (FS_BUFFER_PACKED | flags) },    /* Destination address. */
        {(uint8_t []){0},   1, flags },                         /* Zero. */
        {&protocol,         1, flags },                         /* Protocol. */
        {&length,           2, (FS_BUFFER_PACKED | flags) },    /* Packet length. */
    };

    SYS_LOG_FUNTION_ENTRY(NET_CSUM);

    /* Allocate a buffer and initialize a pseudo header. */
    csum_buffer = fs_buffer_get(buffer->fd, FS_BUFFER_LIST, flags);

    /* If we have to buffer to compute checksum. */
    if (csum_buffer != NULL)
    {
        /* Initialize header generator machine. */
        header_gen_machine_init(&hdr_machine, &fs_buffer_hdr_push);

        /* Push the pseudo header on the buffer. */
        status = header_generate(&hdr_machine, pseudo_hdr, sizeof(pseudo_hdr)/sizeof(HEADER), csum_buffer);

        /* If pseudo header was successfully generated. */
        if (status == SUCCESS)
        {
            /* Calculate and return the checksum for the pseudo header. */
            ret_csum = net_csum_calculate(csum_buffer, -1, 0);

            /* Calculate and add the checksum for actual packet. */
            NET_CSUM_ADD(ret_csum, net_csum_calculate(buffer, -1, offset));

            /* Return the calculated checksum. */
            *csum = (uint16_t)ret_csum;
        }

        /* Free the pseudo header buffer. */
        fs_buffer_add(buffer->fd, csum_buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
    }
    else
    {
        /* There are no buffers. */
        status = FS_BUFFER_NO_SPACE;
    }

    SYS_LOG_FUNTION_EXIT(NET_CSUM);

    /* Return status to the caller. */
    return (status);

} /* net_pseudo_csum_calculate */

/*
 * net_csum_calculate
 * @buffer: File system buffer for which checksum is needed to be calculated.
 * @num_bytes: Number of bytes for which the checksum is needed to be calculated.
 * @offset: Number of bytes we need to skip.
 * This function will calculate and return the check of all the data in the
 * given buffer.
 */
uint16_t net_csum_calculate(FS_BUFFER *buffer, int32_t num_bytes, uint32_t offset)
{
    FS_BUFFER_ONE *one = buffer->list.head;
    uint32_t csum = 0, left = 0;
    uint16_t *bytes = (uint16_t *)one->buffer;
    uint8_t last_left = FALSE;

    SYS_LOG_FUNTION_ENTRY(NET_CSUM);

    /* If we have negative number of bytes. */
    if (num_bytes < 0)
    {
        /* Should never happen. */
        OS_ASSERT(offset > buffer->total_length);

        /* Return the checksum for all the data in the buffer. */
        num_bytes = (int32_t)(buffer->total_length - offset);
    }
    else
    {
        /* Should never happen. */
        OS_ASSERT((offset + (uint32_t)num_bytes) > buffer->total_length);
    }

    /* Move over the offset. */
    while ((one) && (offset > 0))
    {
        /* If this buffer has more data to process. */
        if (one->length > offset)
        {
            /* Pick the number of bytes we have in this buffer. */
            left = (one->length - offset);

            /* Pick the pointer to the data for which checksum is needed. */
            bytes = (uint16_t *)&one->buffer[offset];

            /* Break out of this loop. */
            break;
        }
        else
        {
            /* Update the remaining offset. */
            offset -= one->length;
        }

        /* Pick the next buffer. */
        one = one->next;
    }

    while ((one) && (num_bytes > 0))
    {
        /* If we need to get this buffer information. */
        if (left == 0)
        {
            /* Pick the number of bytes we have in this buffer. */
            left = one->length;
            bytes = (uint16_t *)one->buffer;
        }

        /* If this buffer has more data than required. */
        if ((int32_t)left > num_bytes)
        {
            /* Only process required number of bytes. */
            left = (uint32_t)num_bytes;
            num_bytes = 0;
        }
        else
        {
            /* Decrement the number of bytes we will process from this buffer. */
            num_bytes -= (int32_t)left;
        }

        /* If we have a byte left from the last buffer. */
        if (last_left == TRUE)
        {
            /* If we have at-least a byte in this buffer. */
            if (left > 0)
            {
                /* Add first byte in checksum. */
#ifdef OS_LITTLE_ENDIAN
                csum += (uint16_t)(*((uint8_t *)bytes) << 8);
#else
                csum += *((uint8_t *)bytes);
#endif
                bytes = (uint16_t *)((uint8_t *)bytes + 1);

                /* Reset the last left flag. */
                last_left = FALSE;

                /* Remove the last byte. */
                left -= 1;
            }
        }

        /* While we have data in this buffer. */
        while (left >= 2)
        {
            /* Add data from this buffer. */
            csum += *bytes;
            bytes++;

            /* Decrement number of bytes left to process. */
            left = (uint32_t)(left - 2);
        }

        /* If we still have a byte left on this buffer. */
        if (left == 1)
        {
            /* Add last byte in checksum. */
#ifdef OS_LITTLE_ENDIAN
            csum += *((uint8_t *)bytes);
#else
            csum += (uint16_t)(*((uint8_t *)bytes) << 8);
#endif

            /* We we still need to process next byte. */
            if (one ->next != NULL)
            {
                /* We need to add first byte from the next buffer as it is. */
                last_left = TRUE;
            }

            /* Remove the last byte. */
            left -= 1;
        }

        /* Pick the next buffer. */
        one = one->next;
    }

    /* Add back the carry to the checksum. */
    csum = (csum & 0xFFFF) + (csum >> 16);
    csum = (csum & 0xFFFF) + (csum >> 16);

    SYS_LOG_FUNTION_EXIT(NET_CSUM);

    /* Take one's complement of the checksum. */
    return (~(csum) & 0xFFFF);

} /* net_csum_calculate */

#endif /* CONFIG_NET */
