/*
 * nmea.c
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
#include <kernel.h>

#ifdef CONFIG_NMEA
#include <nmea.h>

/* Helper definitions. */
static uint32_t nmea_pow10_lookup[] = {1, 10, 100, 1000, 10000, 100000};

/*
 * nmea_parser_set_value
 * @value: Integer value will be updated here.
 * @index: Maintains the current index.
 * @have_dot: If we are parsing the decimal places.
 * @chr: Current character.
 * @num_decimal: Number of expected decimal places.
 * Helper function to convert ASII decimal numbers to integers.
 */
void nmea_parser_set_value(uint32_t *value, uint8_t *index, uint8_t *have_dot, uint8_t chr, uint8_t num_decimal)
{
    /* If we do not have a decimal point. */
    if (chr != '.')
    {
        /* If we are adding fractions. */
        if (*have_dot)
        {
            /* While we have an expected deimal place. */
            if ((*index) > 0)
            {
                /* Update the value. */
                (*value) += ((uint32_t)(chr - '0')) * nmea_pow10_lookup[(*index) - 1];

                /* Decrement the index. */
                (*index) --;
            }
        }
        else
        {
            /* Left shift the value in decimal. */
            (*value) *= (uint32_t)10;

            /* Add the new value. */
            (*value) += ((uint32_t)(chr - '0') * nmea_pow10_lookup[num_decimal]);

            /* Increment the index. */
            (*index) ++;
        }
    }
    else
    {
        /* See the flag that we have a dot. */
        (*have_dot) = TRUE;

        /* Reset the index. */
        (*index) = num_decimal;
    }
} /* nmea_parser_set_value */

#endif /* CONFIG_NMEA */
