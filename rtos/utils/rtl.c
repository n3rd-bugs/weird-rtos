/*
 * rtl.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <rtl.h>
#include <kernel.h>

/*
 * rtl_ultoa
 * @in: 32-bit integer needed to be converted to ASCII.
 * @out: Buffer in which result will be placed.
 * @max: Maximum expected value.
 * @flags: Flags to specify operation.
 * This function converts a 32 bit integer in to an ASCII representation with base 10.
 */
void rtl_ultoa(uint32_t in, uint8_t *out, uint32_t max, uint8_t flags)
{
    uint32_t div = RTL_ULTOA_FIRST_DIV;
    uint8_t this_digit = 0;
    uint8_t not_the_first = FALSE;

    /* If we have a maximum value; */
    if (max != 0)
    {
        /* If input is greater than max. */
        if (in > max)
        {
            /* Truncate input. */
            in = max;
        }

        /* While we have required divider. */
        while (div > max)
        {
            div = div / 10;
        }
    }

    /* If need to add leading zeros. */
    if (flags & RTL_ULTOA_LEADING_ZEROS)
    {
        /* Just set not the first flag. */
        not_the_first = TRUE;
    }

    /* Pick the first non zero digit. */
    for (;;)
    {
        /* Calculate the digit. */
        this_digit = (uint8_t)(in / div);

        /* If we need to put this in output string. */
        if ((this_digit) || (not_the_first))
        {
            /* Put the first non zero digit. */
            *out = (uint8_t)('0' + this_digit);
            out++;

            /* We just added something. */
            not_the_first = TRUE;
        }

        /* If this was the last division. */
        if (div == 1)
        {
            /* Break out of the loop. */
            break;
        }

        /* Pick the rest of the number. */
        in = in % div;

        /* Update the deviser. */
        div = div / 10;
    }

    /* If we did not put anything in the output. */
    if (!not_the_first)
    {
        /* Value must be zero. */
        *out = '0';
        out++;
    }

    /* Null terminate the output. */
    *out = '\0';

} /* rtl_ultoa */
