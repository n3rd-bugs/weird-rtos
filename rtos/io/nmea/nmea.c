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
 * nmea_fetch_data
 * @nmea: NMEA instance.
 * @req_msg: Bit field to specify the messages to receive.
 * This will receive the requested messages and populate the data in the provided
 * NMEA instance. Caller is responsible for not reading the stale data from the
 * NMEA instance.
 */
int32_t nmea_fetch_data(NMEA *nmea, uint8_t req_msg)
{
    int32_t status = SUCCESS;
    uint8_t this_msg;
    uint8_t msg_received = 0;

    /* Loop until we have received all the messages. */
    for ( ;((status == SUCCESS) && ((req_msg & msg_received) != req_msg)); )
    {
        /* Parse a message from the bus. */
        status = nmea_parse_message(nmea, NULL, &this_msg);

        if (status == SUCCESS)
        {
            /* Update bit field for this message. */
            msg_received |= this_msg;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* nmea_fetch_data */

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
            /* While we have an expected decimal place. */
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
            /* If not a start of new value. */
            if (*index != 0)
            {
                /* Left shift the value in decimal. */
                (*value) *= (uint32_t)10;
            }
            else
            {
                /* Reset the value. */
                (*value) = 0;
            }

            /* Add the new value. */
            (*value) += ((uint32_t)(chr - '0') * nmea_pow10_lookup[num_decimal]);

            /* Increment the index. */
            (*index) ++;
        }
    }
    else
    {
        /* Set the flag that we have a dot. */
        (*have_dot) = TRUE;

        /* If there was no integral part. */
        if ((*index) == 0)
        {
            /* Reset the value. */
            (*value) = 0;
        }

        /* Reset the index. */
        (*index) = num_decimal;
    }
} /* nmea_parser_set_value */
#endif /* CONFIG_NMEA */
