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
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>

#ifdef CONFIG_NET
#include <net.h>
#include <net_csum.h>

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
    uint16_t *bytes;
    uint8_t last_left = FALSE;

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

    /* Moveover the offset. */
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

    /* Take one's complement of the checksum. */
    return (~(csum) & 0xFFFF);

} /* net_csum_calculate */

#endif /* CONFIG_NET */
