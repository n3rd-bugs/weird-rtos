/* If filename is nmea_parser.c than it is generated from nmea_parser.rl.
 * please see README.md for more details.
 * Do not edit! */
/*
 * nmea_parser.rl
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
#include <fs.h>
#include <fs_buffer.h>

%%{
    # Define machine.
    machine nmea;

    action csum_reset
    {
        /* Reset message checksum. */
        csum = 0;
    }
    action csum_set
    {
        /* Save message checksum. */
        csum_got = (uint8_t)(csum_got << (4 * index));
        csum_got |= (uint8_t)(*p - ((*p > '9') ? ('A' - 10) : '0'));
        index ++;
    }
    action csum_computed
    {
        /* Save the computed checksum. */
        csum_computed = csum;
    }
    action index_reset
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    action talker_set
    {
        /* Save the talker ID. */
        msg->talker_id[index != 0] = *p;
        index++;
    }

    # Common fields.
    action utc_set
    {
        /* Update UTC */
        nmea_parser_set_value(&msg->utc, &index, &have_dot, *p, 3);
    }
    action lat_set
    {
        /* Update Latitude */
        nmea_parser_set_value(&msg->latitude, &index, &have_dot, *p, 5);
    }
    action lat_ns_set
    {
        /* Save latitude N/S. */
        msg->latitude_ns = *p;
    }
    action lon_set
    {
        /* Update Longitude */
        nmea_parser_set_value(&msg->longitude, &index, &have_dot, *p, 5);
    }
    action speed_k_set
    {
        /* Update speed knots */
        nmea_parser_set_value(&msg->speed_knots, &index, &have_dot, *p, 3);
    }
    action speed_mph_set
    {
        /* Update speed meter p/h */
        nmea_parser_set_value(&msg->speed_mph, &index, &have_dot, *p, 3);
    }
    action course_set
    {
        /* Update course */
        nmea_parser_set_value(&msg->course, &index, &have_dot, *p, 3);
    }
    action date_set
    {
        /* Update date */
        nmea_parser_set_value(&msg->date, &index, &have_dot, *p, 0);
    }
    action lon_ew_set
    {
        /* Save longitude E/W. */
        msg->longitude_ew = *p;
    }
    action status_set
    {
        /* Save the data status. */
        msg->status = *p;
    }
    action mode_set
    {
        /* Save the data mode. */
        msg->mode = *p;
    }

    # GGA definitions.
    action got_gga
    {
        /* Save the message ID. */
        msg->id = NMEA_MSG_GGA;
    }
    action fix_set
    {
        msg->data.gaa.fix = (uint8_t)(*p - '0');
    }
    action sat_used_set
    {
        msg->data.gaa.used = (uint8_t)(msg->data.gaa.used * 10);
        msg->data.gaa.used = (uint8_t)(*p - '0' + msg->data.gaa.used);
    }
    action hdop_set
    {
        /* Update HDOP. */
        nmea_parser_set_value(&msg->data.gaa.hdop, &index, &have_dot, *p, 3);
    }
    action alt_set
    {
        /* Update altitude. */
        nmea_parser_set_value(&msg->data.gaa.altitude, &index, &have_dot, *p, 3);
    }
    action alt_unit_set
    {
        /* Save the altitude units. */
        msg->data.gaa.alt_unit = *p;
    }
    action geoid_neg_set
    {
        /* Set the GEOID as negative. */
        msg->data.gaa.geoid_neg = TRUE;
    }
    action geoid_set
    {
        /* Update GEOID. */
        nmea_parser_set_value(&msg->data.gaa.geoid_sep, &index, &have_dot, *p, 3);
    }
    action geoid_unit_set
    {
        /* Save the GEOID units. */
        msg->data.gaa.geoid_unit = *p;
    }

    # GLL definitions.
    action got_gll
    {
        /* Save the message ID. */
        msg->id = NMEA_MSG_GLL;
    }

    # RMC definitions.
    action got_rmc
    {
        /* Save the message ID. */
        msg->id = NMEA_MSG_RMC;
    }

    # VTG definitions.
    action got_vtg
    {
        /* Save the message ID. */
        msg->id = NMEA_MSG_VTG;
    }

    # GSA definitions.
    action got_gsa
    {
        /* Save the message ID. */
        msg->id = NMEA_MSG_GSA;
    }

    # GSV definitions.
    action got_gsv
    {
        /* Save the message ID. */
        msg->id = NMEA_MSG_GSV;
    }

    # Start of a message.
    start = (alnum|[,.*\r\n])* '$'{1} @csum_reset;

    # Match and save talker.
    talker = ((upper)@talker_set){2};

    # Match GGA message.
    gga = ('G''G''A')@got_gga
          ','@index_reset(((digit|[.])@utc_set)+){,1}       # UTC
          ','@index_reset(((digit|[.])@lat_set)+){,1}       # Latitude
          ','([NS]@lat_ns_set){,1}                          # N/S
          ','@index_reset(((digit|[.])@lon_set)+){,1}       # Longitude
          ',' ([EW]@lon_ew_set){,1}                         # E/W
          ','(digit@fix_set){,1}                            # Fix Indicator
          ','@index_reset((digit@sat_used_set){1,2}){,1}    # Setallites used
          ','@index_reset(((digit|[.])@hdop_set)+){,1}      # HDOP
          ','@index_reset(((digit|[.])@alt_set)+){,1}       # MSL Altitude
          ','(alpha@alt_unit_set){,1}                       # MSL units
          ','(('-')@geoid_neg_set){,1}                      # GEOID +/-
             @index_reset(((digit|[.])@geoid_set)+){,1}     # GEOID seperation
          ','(alpha@geoid_unit_set){,1}                     # GEOID units
          ','((digit|[.])+){,1}                             # Age of Diff. Corr.
          ','((digit|[.])+){,1};                            # Diff. Ref. Station ID

    # Match GLL message.
    gll = ('G''L''L')@got_gll
          ','@index_reset(((digit|[.])@lat_set)+){,1}       # Latitude
          ','([NS]@lat_ns_set){,1}                          # N/S
          ','@index_reset(((digit|[.])@lon_set)+){,1}       # Longitude
          ',' ([EW]@lon_ew_set){,1}                         # E/W
          ','@index_reset(((digit|[.])@utc_set)+){,1}       # UTC
          ','(alpha@status_set){,1}                         # Data status
          ','(alpha@mode_set){,1};                          # Data mode

    # Match RMC message.
    rmc = ('R''M''C')@got_rmc
          ','@index_reset(((digit|[.])@utc_set)+){,1}       # UTC
          ','(alpha@status_set){,1}                         # Data status
          ','@index_reset(((digit|[.])@lat_set)+){,1}       # Latitude
          ','([NS]@lat_ns_set){,1}                          # N/S
          ','@index_reset(((digit|[.])@lon_set)+){,1}       # Longitude
          ',' ([EW]@lon_ew_set){,1}                         # E/W
          ','@index_reset(((digit|[.])@speed_k_set)+){,1}   # Speed Knots
          ','@index_reset(((digit|[.])@course_set)+){,1}    # Course
          ','@index_reset((digit@date_set)+){,1}            # Date
          ','((digit|[.])+){,1}                             # Magnetic Variation
          ','((digit|[.])+){,1}                             # East/West Indicator
          ','(alpha@mode_set){,1};                          # Data mode

    # Match VTG message.
    vtg = ('V''T''G')@got_vtg
          ','@index_reset(((digit|[.])@course_set)+){,1}    # Course
          ',' ('T'){,1}                                     # T
          ','((digit|[.])+){,1}                             # Magnetic Course
          ',' ('M'){,1}                                     # M
          ','@index_reset(((digit|[.])@speed_k_set)+){,1}   # Speed knots
          ',' ('N'){,1}                                     # N
          ','@index_reset(((digit|[.])@speed_mph_set)+){,1} # Speed meter per hour
          ',' ('K'){,1}                                     # K
          ','(alpha@mode_set){,1};                          # Data mode

    # Discard GSA.
    gsa = ('G''S''A')@got_gsa
          (','(alnum|[.])*)*;                               # Discard all the arguments

    # Discard GSV.
    gsv = ('G''S''V')@got_gsv
          (','(alnum|[.])*)*;                               # Discard all the arguments

    # Machine entry.
    main := start@index_reset talker (gga|gll|rmc|vtg|gsa|gsv) '*'@csum_computed ((digit|/[A-F]/)@csum_set){2}>index_reset'\r''\n';
}%%

/* Machine definitions. */
%% write data;

/*
 * nmea_parse_message
 * @nmea: NMEA instance.
 * @msg: Parsed message will be returned here.
 * @return: Success will be returned if a message was successfully parsed,
 *  NMEA_READ_ERROR will be returned if an error occurred while reading from
 *      file descriptor,
 *  NMEA_SEQUENCE_ERROR will be returned if an invalid sequence was detected,
 *  NMEA_CSUM_ERROR will be returned if checksum was not valid.
 * This function will return a parsed reading from a NMEA bus/device.
 */
int32_t nmea_parse_message(NMEA *nmea, NMEA_MSG *msg)
{
    int32_t status = SUCCESS;
    uint8_t chr[2], index, have_dot;
    uint8_t *p = &chr[0];
    uint8_t *pe = &chr[1];
    uint8_t csum = 0;
    uint8_t csum_got = 0;
    uint8_t csum_computed = 0;
    char cs = nmea_start;
    FS *fs = (FS *)nmea->fd;
    FS_BUFFER_LIST *buffer = NULL;

    /* Remove some compiler warning. */
    UNUSED_PARAM(nmea_en_main);

    for (;;)
    {
        /* If this is a buffer file descriptor. */
        if (fs->flags & FS_BUFFERED)
        {
            /* If we don't have any data to process. */
            if ((buffer == NULL) || (buffer->total_length == 0))
            {
                /* If we have a last buffer. */
                if (buffer != NULL)
                {
                    /* Lock the file descriptor. */
                    fd_get_lock(nmea->fd);

                    /* Free the last buffer. */
                    fs_buffer_add(nmea->fd, buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);

                    /* Release file descriptor lock. */
                    fd_release_lock(nmea->fd);
                }

                /* Read a byte from the stream. */
                status = fs_read(nmea->fd, (void *)&buffer, sizeof(buffer));
            }

            /* If we do have a valid buffer. */
            if ((status == sizeof(buffer)) && (buffer != NULL) && (buffer->total_length > 0))
            {
                /* Lock the file descriptor. */
                fd_get_lock(nmea->fd);

                /* Pull a byte from the buffer. */
                (void)fs_buffer_list_pull(buffer, chr, 1, FS_BUFFER_HEAD);

                /* Release file descriptor lock. */
                fd_release_lock(nmea->fd);
            }
            else
            {
                /* If we did not read expected data. */
                if (status >= 0)
                {
                    /* Return error to the caller. */
                    status = NMEA_READ_ERROR;
                }

                break;
            }
        }
        else
        {
            /* Read a byte from the stream. */
            status = fs_read(nmea->fd, chr, 1);

            if (status != 1)
            {
                /* Return error to the caller. */
                status = NMEA_READ_ERROR;

                break;
            }
        }

        /* Reset the read pointer. */
        p = chr;

        /* Update the checksum. */
        if (*p != '*')
        {
            csum ^= *p;
        }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
#pragma GCC diagnostic ignored "-Wchar-subscripts"
#pragma GCC diagnostic ignored "-Wsign-conversion"
        %% write exec;
#pragma GCC diagnostic pop

        /* Check if machine is now in finished state. */
        if ((cs == nmea_first_final) || (cs == nmea_error))
        {
            /* State machine error. */
            if (cs == nmea_error)
            {
                /* Return error that invalid sequence was read. */
                status = NMEA_SEQUENCE_ERROR;
            }

            /* If checksum does not match. */
            else if (csum_got != csum_computed)
            {
                /* Return error that checksum did not match. */
                status = NMEA_CSUM_ERROR;
            }
            
            /* Everything is okay. */
            else
            {
                /* Return success. */
                status = SUCCESS;
            }

            break;
        }
    }

    /* If we allocated a buffer. */
    if (buffer != NULL)
    {
        /* Lock the file descriptor. */
        fd_get_lock(nmea->fd);

        /* Free the last buffer. */
        fs_buffer_add(nmea->fd, buffer, FS_LIST_FREE, FS_BUFFER_ACTIVE);

        /* Release file descriptor lock. */
        fd_release_lock(nmea->fd);
    }

    /* Return status to the caller. */
    return (status);

} /* nmea_parse_message */

#endif /* CONFIG_NMEA */
