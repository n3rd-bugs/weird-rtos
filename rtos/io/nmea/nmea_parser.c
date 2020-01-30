
/* #line 1 "nmea_parser.rl" */
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


/* #line 266 "nmea_parser.rl" */


/* Machine definitions. */

/* #line 32 "nmea_parser.c" */
static const int nmea_start = 1;
static const int nmea_first_final = 81;
static const int nmea_error = 0;

static const int nmea_en_main = 1;


/* #line 270 "nmea_parser.rl" */

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
        
/* #line 146 "nmea_parser.c" */
    {
    if ( p == pe )
        goto _test_eof;
    switch ( cs )
    {
st1:
    if ( ++p == pe )
        goto _test_eof1;
case 1:
    switch( (*p) ) {
        case 10: goto st1;
        case 13: goto st1;
        case 36: goto tr2;
        case 42: goto st1;
        case 44: goto st1;
        case 46: goto st1;
    }
    if ( (*p) < 65 ) {
        if ( 48 <= (*p) && (*p) <= 57 )
            goto st1;
    } else if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto st1;
    } else
        goto st1;
    goto st0;
st0:
cs = 0;
    goto _out;
tr2:
/* #line 28 "nmea_parser.rl" */
    {
        /* Reset message checksum. */
        csum = 0;
    }
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st2;
st2:
    if ( ++p == pe )
        goto _test_eof2;
case 2:
/* #line 193 "nmea_parser.c" */
    if ( 65 <= (*p) && (*p) <= 90 )
        goto tr3;
    goto st0;
tr3:
/* #line 51 "nmea_parser.rl" */
    {
        /* Save the talker ID. */
        msg->talker_id[index != 0] = *p;
        index++;
    }
    goto st3;
st3:
    if ( ++p == pe )
        goto _test_eof3;
case 3:
/* #line 209 "nmea_parser.c" */
    if ( 65 <= (*p) && (*p) <= 90 )
        goto tr4;
    goto st0;
tr4:
/* #line 51 "nmea_parser.rl" */
    {
        /* Save the talker ID. */
        msg->talker_id[index != 0] = *p;
        index++;
    }
    goto st4;
st4:
    if ( ++p == pe )
        goto _test_eof4;
case 4:
/* #line 225 "nmea_parser.c" */
    switch( (*p) ) {
        case 71: goto st5;
        case 82: goto st50;
        case 86: goto st67;
    }
    goto st0;
st5:
    if ( ++p == pe )
        goto _test_eof5;
case 5:
    switch( (*p) ) {
        case 71: goto st6;
        case 76: goto st34;
        case 83: goto st47;
    }
    goto st0;
st6:
    if ( ++p == pe )
        goto _test_eof6;
case 6:
    if ( (*p) == 65 )
        goto tr11;
    goto st0;
tr11:
/* #line 116 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        msg->msg_id = NMEA_MSG_GGA;
    }
    goto st7;
st7:
    if ( ++p == pe )
        goto _test_eof7;
case 7:
/* #line 260 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr12;
    goto st0;
tr12:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st8;
tr14:
/* #line 59 "nmea_parser.rl" */
    {
        /* Update UTC */
        nmea_parser_set_value(&msg->utc, &index, &have_dot, *p, 3);
    }
    goto st8;
st8:
    if ( ++p == pe )
        goto _test_eof8;
case 8:
/* #line 283 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto tr13;
        case 46: goto tr14;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr14;
    goto st0;
tr13:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st9;
tr16:
/* #line 64 "nmea_parser.rl" */
    {
        /* Update Latitude */
        nmea_parser_set_value(&msg->latitude, &index, &have_dot, *p, 5);
    }
    goto st9;
st9:
    if ( ++p == pe )
        goto _test_eof9;
case 9:
/* #line 310 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st10;
        case 46: goto tr16;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr16;
    goto st0;
st10:
    if ( ++p == pe )
        goto _test_eof10;
case 10:
    switch( (*p) ) {
        case 44: goto tr17;
        case 78: goto tr18;
        case 83: goto tr18;
    }
    goto st0;
tr17:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st11;
tr20:
/* #line 74 "nmea_parser.rl" */
    {
        /* Update Longitude */
        nmea_parser_set_value(&msg->longitude, &index, &have_dot, *p, 5);
    }
    goto st11;
st11:
    if ( ++p == pe )
        goto _test_eof11;
case 11:
/* #line 347 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st12;
        case 46: goto tr20;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr20;
    goto st0;
st12:
    if ( ++p == pe )
        goto _test_eof12;
case 12:
    switch( (*p) ) {
        case 44: goto st13;
        case 69: goto tr22;
        case 87: goto tr22;
    }
    goto st0;
st13:
    if ( ++p == pe )
        goto _test_eof13;
case 13:
    if ( (*p) == 44 )
        goto tr23;
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr24;
    goto st0;
tr23:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st14;
st14:
    if ( ++p == pe )
        goto _test_eof14;
case 14:
/* #line 386 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr25;
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr26;
    goto st0;
tr25:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st15;
tr28:
/* #line 130 "nmea_parser.rl" */
    {
        /* Update HDOP. */
        nmea_parser_set_value(&msg->data.gaa.hdop, &index, &have_dot, *p, 3);
    }
    goto st15;
st15:
    if ( ++p == pe )
        goto _test_eof15;
case 15:
/* #line 411 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto tr27;
        case 46: goto tr28;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr28;
    goto st0;
tr27:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st16;
tr30:
/* #line 135 "nmea_parser.rl" */
    {
        /* Update altitude. */
        nmea_parser_set_value(&msg->data.gaa.altitude, &index, &have_dot, *p, 3);
    }
    goto st16;
st16:
    if ( ++p == pe )
        goto _test_eof16;
case 16:
/* #line 438 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st17;
        case 46: goto tr30;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr30;
    goto st0;
st17:
    if ( ++p == pe )
        goto _test_eof17;
case 17:
    if ( (*p) == 44 )
        goto st18;
    if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto tr32;
    } else if ( (*p) >= 65 )
        goto tr32;
    goto st0;
st18:
    if ( ++p == pe )
        goto _test_eof18;
case 18:
    switch( (*p) ) {
        case 44: goto st19;
        case 45: goto tr34;
        case 46: goto tr35;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr35;
    goto st0;
st19:
    if ( ++p == pe )
        goto _test_eof19;
case 19:
    if ( (*p) == 44 )
        goto st20;
    if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto tr37;
    } else if ( (*p) >= 65 )
        goto tr37;
    goto st0;
st20:
    if ( ++p == pe )
        goto _test_eof20;
case 20:
    switch( (*p) ) {
        case 44: goto st21;
        case 46: goto st20;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto st20;
    goto st0;
st21:
    if ( ++p == pe )
        goto _test_eof21;
case 21:
    switch( (*p) ) {
        case 42: goto tr39;
        case 46: goto st21;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto st21;
    goto st0;
tr39:
/* #line 40 "nmea_parser.rl" */
    {
        /* Save the computed checksum. */
        csum_computed = csum;
    }
    goto st22;
st22:
    if ( ++p == pe )
        goto _test_eof22;
case 22:
/* #line 515 "nmea_parser.c" */
    if ( (*p) > 57 ) {
        if ( 65 <= (*p) && (*p) <= 70 )
            goto tr40;
    } else if ( (*p) >= 48 )
        goto tr40;
    goto st0;
tr40:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
/* #line 33 "nmea_parser.rl" */
    {
        /* Save message checksum. */
        csum_got = (uint8_t)(csum_got << (4 * index));
        csum_got |= (uint8_t)(*p - ((*p > '9') ? ('A' - 10) : '0'));
        index ++;
    }
    goto st23;
st23:
    if ( ++p == pe )
        goto _test_eof23;
case 23:
/* #line 541 "nmea_parser.c" */
    if ( (*p) > 57 ) {
        if ( 65 <= (*p) && (*p) <= 70 )
            goto tr41;
    } else if ( (*p) >= 48 )
        goto tr41;
    goto st0;
tr41:
/* #line 33 "nmea_parser.rl" */
    {
        /* Save message checksum. */
        csum_got = (uint8_t)(csum_got << (4 * index));
        csum_got |= (uint8_t)(*p - ((*p > '9') ? ('A' - 10) : '0'));
        index ++;
    }
    goto st24;
st24:
    if ( ++p == pe )
        goto _test_eof24;
case 24:
/* #line 561 "nmea_parser.c" */
    if ( (*p) == 13 )
        goto st25;
    goto st0;
st25:
    if ( ++p == pe )
        goto _test_eof25;
case 25:
    if ( (*p) == 10 )
        goto st81;
    goto st0;
st81:
    if ( ++p == pe )
        goto _test_eof81;
case 81:
    goto st0;
tr37:
/* #line 155 "nmea_parser.rl" */
    {
        /* Save the GEOID units. */
        msg->data.gaa.geoid_unit = *p;
    }
    goto st26;
st26:
    if ( ++p == pe )
        goto _test_eof26;
case 26:
/* #line 588 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto st20;
    goto st0;
tr34:
/* #line 145 "nmea_parser.rl" */
    {
        /* Set the GEOID as negative. */
        msg->data.gaa.geoid_neg = TRUE;
    }
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st27;
tr35:
/* #line 150 "nmea_parser.rl" */
    {
        /* Update GEOID. */
        nmea_parser_set_value(&msg->data.gaa.geoid_sep, &index, &have_dot, *p, 3);
    }
    goto st27;
st27:
    if ( ++p == pe )
        goto _test_eof27;
case 27:
/* #line 616 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st19;
        case 46: goto tr35;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr35;
    goto st0;
tr32:
/* #line 140 "nmea_parser.rl" */
    {
        /* Save the altitude units. */
        msg->data.gaa.alt_unit = *p;
    }
    goto st28;
st28:
    if ( ++p == pe )
        goto _test_eof28;
case 28:
/* #line 635 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto st18;
    goto st0;
tr26:
/* #line 125 "nmea_parser.rl" */
    {
        msg->data.gaa.used = (uint8_t)(msg->data.gaa.used * 10);
        msg->data.gaa.used = (uint8_t)(*p - '0' + msg->data.gaa.used);
    }
    goto st29;
st29:
    if ( ++p == pe )
        goto _test_eof29;
case 29:
/* #line 650 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr25;
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr44;
    goto st0;
tr44:
/* #line 125 "nmea_parser.rl" */
    {
        msg->data.gaa.used = (uint8_t)(msg->data.gaa.used * 10);
        msg->data.gaa.used = (uint8_t)(*p - '0' + msg->data.gaa.used);
    }
    goto st30;
st30:
    if ( ++p == pe )
        goto _test_eof30;
case 30:
/* #line 667 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr25;
    goto st0;
tr24:
/* #line 121 "nmea_parser.rl" */
    {
        msg->data.gaa.fix = (uint8_t)(*p - '0');
    }
    goto st31;
st31:
    if ( ++p == pe )
        goto _test_eof31;
case 31:
/* #line 681 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr23;
    goto st0;
tr22:
/* #line 99 "nmea_parser.rl" */
    {
        /* Save longitude E/W. */
        msg->longitude_ew = *p;
    }
    goto st32;
st32:
    if ( ++p == pe )
        goto _test_eof32;
case 32:
/* #line 696 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto st13;
    goto st0;
tr18:
/* #line 69 "nmea_parser.rl" */
    {
        /* Save latitude N/S. */
        msg->latitude_ns = *p;
    }
    goto st33;
st33:
    if ( ++p == pe )
        goto _test_eof33;
case 33:
/* #line 711 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr17;
    goto st0;
st34:
    if ( ++p == pe )
        goto _test_eof34;
case 34:
    if ( (*p) == 76 )
        goto tr45;
    goto st0;
tr45:
/* #line 162 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        msg->msg_id = NMEA_MSG_GLL;
    }
    goto st35;
st35:
    if ( ++p == pe )
        goto _test_eof35;
case 35:
/* #line 733 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr46;
    goto st0;
tr46:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st36;
tr48:
/* #line 64 "nmea_parser.rl" */
    {
        /* Update Latitude */
        nmea_parser_set_value(&msg->latitude, &index, &have_dot, *p, 5);
    }
    goto st36;
st36:
    if ( ++p == pe )
        goto _test_eof36;
case 36:
/* #line 756 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st37;
        case 46: goto tr48;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr48;
    goto st0;
st37:
    if ( ++p == pe )
        goto _test_eof37;
case 37:
    switch( (*p) ) {
        case 44: goto tr49;
        case 78: goto tr50;
        case 83: goto tr50;
    }
    goto st0;
tr49:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st38;
tr52:
/* #line 74 "nmea_parser.rl" */
    {
        /* Update Longitude */
        nmea_parser_set_value(&msg->longitude, &index, &have_dot, *p, 5);
    }
    goto st38;
st38:
    if ( ++p == pe )
        goto _test_eof38;
case 38:
/* #line 793 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st39;
        case 46: goto tr52;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr52;
    goto st0;
st39:
    if ( ++p == pe )
        goto _test_eof39;
case 39:
    switch( (*p) ) {
        case 44: goto tr53;
        case 69: goto tr54;
        case 87: goto tr54;
    }
    goto st0;
tr53:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st40;
tr56:
/* #line 59 "nmea_parser.rl" */
    {
        /* Update UTC */
        nmea_parser_set_value(&msg->utc, &index, &have_dot, *p, 3);
    }
    goto st40;
st40:
    if ( ++p == pe )
        goto _test_eof40;
case 40:
/* #line 830 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st41;
        case 46: goto tr56;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr56;
    goto st0;
st41:
    if ( ++p == pe )
        goto _test_eof41;
case 41:
    if ( (*p) == 44 )
        goto st42;
    if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto tr58;
    } else if ( (*p) >= 65 )
        goto tr58;
    goto st0;
st42:
    if ( ++p == pe )
        goto _test_eof42;
case 42:
    if ( (*p) == 42 )
        goto tr39;
    if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto tr59;
    } else if ( (*p) >= 65 )
        goto tr59;
    goto st0;
tr59:
/* #line 109 "nmea_parser.rl" */
    {
        /* Save the data mode. */
        msg->mode = *p;
    }
    goto st43;
st43:
    if ( ++p == pe )
        goto _test_eof43;
case 43:
/* #line 873 "nmea_parser.c" */
    if ( (*p) == 42 )
        goto tr39;
    goto st0;
tr58:
/* #line 104 "nmea_parser.rl" */
    {
        /* Save the data status. */
        msg->status = *p;
    }
    goto st44;
st44:
    if ( ++p == pe )
        goto _test_eof44;
case 44:
/* #line 888 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto st42;
    goto st0;
tr54:
/* #line 99 "nmea_parser.rl" */
    {
        /* Save longitude E/W. */
        msg->longitude_ew = *p;
    }
    goto st45;
st45:
    if ( ++p == pe )
        goto _test_eof45;
case 45:
/* #line 903 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr53;
    goto st0;
tr50:
/* #line 69 "nmea_parser.rl" */
    {
        /* Save latitude N/S. */
        msg->latitude_ns = *p;
    }
    goto st46;
st46:
    if ( ++p == pe )
        goto _test_eof46;
case 46:
/* #line 918 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr49;
    goto st0;
st47:
    if ( ++p == pe )
        goto _test_eof47;
case 47:
    switch( (*p) ) {
        case 65: goto tr60;
        case 86: goto tr61;
    }
    goto st0;
tr60:
/* #line 183 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        msg->msg_id = NMEA_MSG_GSA;
    }
    goto st48;
tr61:
/* #line 190 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        msg->msg_id = NMEA_MSG_GSV;
    }
    goto st48;
st48:
    if ( ++p == pe )
        goto _test_eof48;
case 48:
/* #line 949 "nmea_parser.c" */
    switch( (*p) ) {
        case 42: goto tr39;
        case 44: goto st49;
    }
    goto st0;
st49:
    if ( ++p == pe )
        goto _test_eof49;
case 49:
    switch( (*p) ) {
        case 42: goto tr39;
        case 44: goto st49;
        case 46: goto st49;
    }
    if ( (*p) < 65 ) {
        if ( 48 <= (*p) && (*p) <= 57 )
            goto st49;
    } else if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto st49;
    } else
        goto st49;
    goto st0;
st50:
    if ( ++p == pe )
        goto _test_eof50;
case 50:
    if ( (*p) == 77 )
        goto st51;
    goto st0;
st51:
    if ( ++p == pe )
        goto _test_eof51;
case 51:
    if ( (*p) == 67 )
        goto tr64;
    goto st0;
tr64:
/* #line 169 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        msg->msg_id = NMEA_MSG_RMC;
    }
    goto st52;
st52:
    if ( ++p == pe )
        goto _test_eof52;
case 52:
/* #line 998 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr65;
    goto st0;
tr65:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st53;
tr67:
/* #line 59 "nmea_parser.rl" */
    {
        /* Update UTC */
        nmea_parser_set_value(&msg->utc, &index, &have_dot, *p, 3);
    }
    goto st53;
st53:
    if ( ++p == pe )
        goto _test_eof53;
case 53:
/* #line 1021 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st54;
        case 46: goto tr67;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr67;
    goto st0;
st54:
    if ( ++p == pe )
        goto _test_eof54;
case 54:
    if ( (*p) == 44 )
        goto tr68;
    if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto tr69;
    } else if ( (*p) >= 65 )
        goto tr69;
    goto st0;
tr68:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st55;
tr71:
/* #line 64 "nmea_parser.rl" */
    {
        /* Update Latitude */
        nmea_parser_set_value(&msg->latitude, &index, &have_dot, *p, 5);
    }
    goto st55;
st55:
    if ( ++p == pe )
        goto _test_eof55;
case 55:
/* #line 1060 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st56;
        case 46: goto tr71;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr71;
    goto st0;
st56:
    if ( ++p == pe )
        goto _test_eof56;
case 56:
    switch( (*p) ) {
        case 44: goto tr72;
        case 78: goto tr73;
        case 83: goto tr73;
    }
    goto st0;
tr72:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st57;
tr75:
/* #line 74 "nmea_parser.rl" */
    {
        /* Update Longitude */
        nmea_parser_set_value(&msg->longitude, &index, &have_dot, *p, 5);
    }
    goto st57;
st57:
    if ( ++p == pe )
        goto _test_eof57;
case 57:
/* #line 1097 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st58;
        case 46: goto tr75;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr75;
    goto st0;
st58:
    if ( ++p == pe )
        goto _test_eof58;
case 58:
    switch( (*p) ) {
        case 44: goto tr76;
        case 69: goto tr77;
        case 87: goto tr77;
    }
    goto st0;
tr76:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st59;
tr79:
/* #line 79 "nmea_parser.rl" */
    {
        /* Update speed knots */
        nmea_parser_set_value(&msg->speed_knots, &index, &have_dot, *p, 3);
    }
    goto st59;
st59:
    if ( ++p == pe )
        goto _test_eof59;
case 59:
/* #line 1134 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto tr78;
        case 46: goto tr79;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr79;
    goto st0;
tr78:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st60;
tr81:
/* #line 89 "nmea_parser.rl" */
    {
        /* Update course */
        nmea_parser_set_value(&msg->course, &index, &have_dot, *p, 3);
    }
    goto st60;
st60:
    if ( ++p == pe )
        goto _test_eof60;
case 60:
/* #line 1161 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto tr80;
        case 46: goto tr81;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr81;
    goto st0;
tr80:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st61;
tr83:
/* #line 94 "nmea_parser.rl" */
    {
        /* Update date */
        nmea_parser_set_value(&msg->date, &index, &have_dot, *p, 0);
    }
    goto st61;
st61:
    if ( ++p == pe )
        goto _test_eof61;
case 61:
/* #line 1188 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto st62;
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr83;
    goto st0;
st62:
    if ( ++p == pe )
        goto _test_eof62;
case 62:
    switch( (*p) ) {
        case 44: goto st63;
        case 46: goto st62;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto st62;
    goto st0;
st63:
    if ( ++p == pe )
        goto _test_eof63;
case 63:
    switch( (*p) ) {
        case 44: goto st42;
        case 46: goto st63;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto st63;
    goto st0;
tr77:
/* #line 99 "nmea_parser.rl" */
    {
        /* Save longitude E/W. */
        msg->longitude_ew = *p;
    }
    goto st64;
st64:
    if ( ++p == pe )
        goto _test_eof64;
case 64:
/* #line 1227 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr76;
    goto st0;
tr73:
/* #line 69 "nmea_parser.rl" */
    {
        /* Save latitude N/S. */
        msg->latitude_ns = *p;
    }
    goto st65;
st65:
    if ( ++p == pe )
        goto _test_eof65;
case 65:
/* #line 1242 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr72;
    goto st0;
tr69:
/* #line 104 "nmea_parser.rl" */
    {
        /* Save the data status. */
        msg->status = *p;
    }
    goto st66;
st66:
    if ( ++p == pe )
        goto _test_eof66;
case 66:
/* #line 1257 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr68;
    goto st0;
st67:
    if ( ++p == pe )
        goto _test_eof67;
case 67:
    if ( (*p) == 84 )
        goto st68;
    goto st0;
st68:
    if ( ++p == pe )
        goto _test_eof68;
case 68:
    if ( (*p) == 71 )
        goto tr86;
    goto st0;
tr86:
/* #line 176 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        msg->msg_id = NMEA_MSG_VTG;
    }
    goto st69;
st69:
    if ( ++p == pe )
        goto _test_eof69;
case 69:
/* #line 1286 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr87;
    goto st0;
tr87:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st70;
tr89:
/* #line 89 "nmea_parser.rl" */
    {
        /* Update course */
        nmea_parser_set_value(&msg->course, &index, &have_dot, *p, 3);
    }
    goto st70;
st70:
    if ( ++p == pe )
        goto _test_eof70;
case 70:
/* #line 1309 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st71;
        case 46: goto tr89;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr89;
    goto st0;
st71:
    if ( ++p == pe )
        goto _test_eof71;
case 71:
    switch( (*p) ) {
        case 44: goto st72;
        case 84: goto st80;
    }
    goto st0;
st72:
    if ( ++p == pe )
        goto _test_eof72;
case 72:
    switch( (*p) ) {
        case 44: goto st73;
        case 46: goto st72;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto st72;
    goto st0;
st73:
    if ( ++p == pe )
        goto _test_eof73;
case 73:
    switch( (*p) ) {
        case 44: goto tr93;
        case 77: goto st79;
    }
    goto st0;
tr93:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st74;
tr96:
/* #line 79 "nmea_parser.rl" */
    {
        /* Update speed knots */
        nmea_parser_set_value(&msg->speed_knots, &index, &have_dot, *p, 3);
    }
    goto st74;
st74:
    if ( ++p == pe )
        goto _test_eof74;
case 74:
/* #line 1365 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st75;
        case 46: goto tr96;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr96;
    goto st0;
st75:
    if ( ++p == pe )
        goto _test_eof75;
case 75:
    switch( (*p) ) {
        case 44: goto tr97;
        case 78: goto st78;
    }
    goto st0;
tr97:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st76;
tr100:
/* #line 84 "nmea_parser.rl" */
    {
        /* Update speed meter p/h */
        nmea_parser_set_value(&msg->speed_mph, &index, &have_dot, *p, 3);
    }
    goto st76;
st76:
    if ( ++p == pe )
        goto _test_eof76;
case 76:
/* #line 1401 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st77;
        case 46: goto tr100;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr100;
    goto st0;
st77:
    if ( ++p == pe )
        goto _test_eof77;
case 77:
    switch( (*p) ) {
        case 44: goto st42;
        case 75: goto st44;
    }
    goto st0;
st78:
    if ( ++p == pe )
        goto _test_eof78;
case 78:
    if ( (*p) == 44 )
        goto tr97;
    goto st0;
st79:
    if ( ++p == pe )
        goto _test_eof79;
case 79:
    if ( (*p) == 44 )
        goto tr93;
    goto st0;
st80:
    if ( ++p == pe )
        goto _test_eof80;
case 80:
    if ( (*p) == 44 )
        goto st72;
    goto st0;
    }
    _test_eof1: cs = 1; goto _test_eof;
    _test_eof2: cs = 2; goto _test_eof;
    _test_eof3: cs = 3; goto _test_eof;
    _test_eof4: cs = 4; goto _test_eof;
    _test_eof5: cs = 5; goto _test_eof;
    _test_eof6: cs = 6; goto _test_eof;
    _test_eof7: cs = 7; goto _test_eof;
    _test_eof8: cs = 8; goto _test_eof;
    _test_eof9: cs = 9; goto _test_eof;
    _test_eof10: cs = 10; goto _test_eof;
    _test_eof11: cs = 11; goto _test_eof;
    _test_eof12: cs = 12; goto _test_eof;
    _test_eof13: cs = 13; goto _test_eof;
    _test_eof14: cs = 14; goto _test_eof;
    _test_eof15: cs = 15; goto _test_eof;
    _test_eof16: cs = 16; goto _test_eof;
    _test_eof17: cs = 17; goto _test_eof;
    _test_eof18: cs = 18; goto _test_eof;
    _test_eof19: cs = 19; goto _test_eof;
    _test_eof20: cs = 20; goto _test_eof;
    _test_eof21: cs = 21; goto _test_eof;
    _test_eof22: cs = 22; goto _test_eof;
    _test_eof23: cs = 23; goto _test_eof;
    _test_eof24: cs = 24; goto _test_eof;
    _test_eof25: cs = 25; goto _test_eof;
    _test_eof81: cs = 81; goto _test_eof;
    _test_eof26: cs = 26; goto _test_eof;
    _test_eof27: cs = 27; goto _test_eof;
    _test_eof28: cs = 28; goto _test_eof;
    _test_eof29: cs = 29; goto _test_eof;
    _test_eof30: cs = 30; goto _test_eof;
    _test_eof31: cs = 31; goto _test_eof;
    _test_eof32: cs = 32; goto _test_eof;
    _test_eof33: cs = 33; goto _test_eof;
    _test_eof34: cs = 34; goto _test_eof;
    _test_eof35: cs = 35; goto _test_eof;
    _test_eof36: cs = 36; goto _test_eof;
    _test_eof37: cs = 37; goto _test_eof;
    _test_eof38: cs = 38; goto _test_eof;
    _test_eof39: cs = 39; goto _test_eof;
    _test_eof40: cs = 40; goto _test_eof;
    _test_eof41: cs = 41; goto _test_eof;
    _test_eof42: cs = 42; goto _test_eof;
    _test_eof43: cs = 43; goto _test_eof;
    _test_eof44: cs = 44; goto _test_eof;
    _test_eof45: cs = 45; goto _test_eof;
    _test_eof46: cs = 46; goto _test_eof;
    _test_eof47: cs = 47; goto _test_eof;
    _test_eof48: cs = 48; goto _test_eof;
    _test_eof49: cs = 49; goto _test_eof;
    _test_eof50: cs = 50; goto _test_eof;
    _test_eof51: cs = 51; goto _test_eof;
    _test_eof52: cs = 52; goto _test_eof;
    _test_eof53: cs = 53; goto _test_eof;
    _test_eof54: cs = 54; goto _test_eof;
    _test_eof55: cs = 55; goto _test_eof;
    _test_eof56: cs = 56; goto _test_eof;
    _test_eof57: cs = 57; goto _test_eof;
    _test_eof58: cs = 58; goto _test_eof;
    _test_eof59: cs = 59; goto _test_eof;
    _test_eof60: cs = 60; goto _test_eof;
    _test_eof61: cs = 61; goto _test_eof;
    _test_eof62: cs = 62; goto _test_eof;
    _test_eof63: cs = 63; goto _test_eof;
    _test_eof64: cs = 64; goto _test_eof;
    _test_eof65: cs = 65; goto _test_eof;
    _test_eof66: cs = 66; goto _test_eof;
    _test_eof67: cs = 67; goto _test_eof;
    _test_eof68: cs = 68; goto _test_eof;
    _test_eof69: cs = 69; goto _test_eof;
    _test_eof70: cs = 70; goto _test_eof;
    _test_eof71: cs = 71; goto _test_eof;
    _test_eof72: cs = 72; goto _test_eof;
    _test_eof73: cs = 73; goto _test_eof;
    _test_eof74: cs = 74; goto _test_eof;
    _test_eof75: cs = 75; goto _test_eof;
    _test_eof76: cs = 76; goto _test_eof;
    _test_eof77: cs = 77; goto _test_eof;
    _test_eof78: cs = 78; goto _test_eof;
    _test_eof79: cs = 79; goto _test_eof;
    _test_eof80: cs = 80; goto _test_eof;

    _test_eof: {}
    _out: {}
    }

/* #line 375 "nmea_parser.rl" */
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
            else if (csum_got == csum_computed)
            {
                /* Return error that checksum did not match. */
                status = NMEA_CSUM_ERROR;
            }

            break;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* nmea_parse_message */

#endif /* CONFIG_NMEA */
