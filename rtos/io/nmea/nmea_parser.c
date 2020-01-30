
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


/* #line 183 "nmea_parser.rl" */


/* Machine definitions. */

/* #line 32 "nmea_parser.c" */
static const int nmea_start = 1;
static const int nmea_first_final = 47;
static const int nmea_error = 0;

static const int nmea_en_main = 1;


/* #line 187 "nmea_parser.rl" */

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
    if ( (*p) == 71 )
        goto st5;
    goto st0;
st5:
    if ( ++p == pe )
        goto _test_eof5;
case 5:
    switch( (*p) ) {
        case 71: goto st6;
        case 76: goto st34;
    }
    goto st0;
st6:
    if ( ++p == pe )
        goto _test_eof6;
case 6:
    if ( (*p) == 65 )
        goto tr8;
    goto st0;
tr8:
/* #line 86 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        msg->msg_id = NMEA_MSG_GGA;
    }
    goto st7;
st7:
    if ( ++p == pe )
        goto _test_eof7;
case 7:
/* #line 256 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr9;
    goto st0;
tr9:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st8;
tr11:
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
/* #line 279 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto tr10;
        case 46: goto tr11;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr11;
    goto st0;
tr10:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st9;
tr13:
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
/* #line 306 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st10;
        case 46: goto tr13;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr13;
    goto st0;
st10:
    if ( ++p == pe )
        goto _test_eof10;
case 10:
    switch( (*p) ) {
        case 44: goto tr14;
        case 78: goto tr15;
        case 83: goto tr15;
    }
    goto st0;
tr14:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st11;
tr17:
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
/* #line 343 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st12;
        case 46: goto tr17;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr17;
    goto st0;
st12:
    if ( ++p == pe )
        goto _test_eof12;
case 12:
    switch( (*p) ) {
        case 44: goto st13;
        case 69: goto tr19;
        case 87: goto tr19;
    }
    goto st0;
st13:
    if ( ++p == pe )
        goto _test_eof13;
case 13:
    if ( (*p) == 44 )
        goto tr20;
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr21;
    goto st0;
tr20:
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
/* #line 382 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr22;
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr23;
    goto st0;
tr22:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st15;
tr25:
/* #line 100 "nmea_parser.rl" */
    {
        /* Update HDOP. */
        nmea_parser_set_value(&msg->data.gaa.hdop, &index, &have_dot, *p, 3);
    }
    goto st15;
st15:
    if ( ++p == pe )
        goto _test_eof15;
case 15:
/* #line 407 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto tr24;
        case 46: goto tr25;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr25;
    goto st0;
tr24:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st16;
tr27:
/* #line 105 "nmea_parser.rl" */
    {
        /* Update altitude. */
        nmea_parser_set_value(&msg->data.gaa.altitude, &index, &have_dot, *p, 3);
    }
    goto st16;
st16:
    if ( ++p == pe )
        goto _test_eof16;
case 16:
/* #line 434 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st17;
        case 46: goto tr27;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr27;
    goto st0;
st17:
    if ( ++p == pe )
        goto _test_eof17;
case 17:
    if ( (*p) == 44 )
        goto st18;
    if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto tr29;
    } else if ( (*p) >= 65 )
        goto tr29;
    goto st0;
st18:
    if ( ++p == pe )
        goto _test_eof18;
case 18:
    switch( (*p) ) {
        case 44: goto st19;
        case 45: goto tr31;
        case 46: goto tr32;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr32;
    goto st0;
st19:
    if ( ++p == pe )
        goto _test_eof19;
case 19:
    if ( (*p) == 44 )
        goto st20;
    if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto tr34;
    } else if ( (*p) >= 65 )
        goto tr34;
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
        case 42: goto tr36;
        case 46: goto st21;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto st21;
    goto st0;
tr36:
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
/* #line 511 "nmea_parser.c" */
    if ( (*p) > 57 ) {
        if ( 65 <= (*p) && (*p) <= 70 )
            goto tr37;
    } else if ( (*p) >= 48 )
        goto tr37;
    goto st0;
tr37:
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
/* #line 537 "nmea_parser.c" */
    if ( (*p) > 57 ) {
        if ( 65 <= (*p) && (*p) <= 70 )
            goto tr38;
    } else if ( (*p) >= 48 )
        goto tr38;
    goto st0;
tr38:
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
/* #line 557 "nmea_parser.c" */
    if ( (*p) == 13 )
        goto st25;
    goto st0;
st25:
    if ( ++p == pe )
        goto _test_eof25;
case 25:
    if ( (*p) == 10 )
        goto st47;
    goto st0;
st47:
    if ( ++p == pe )
        goto _test_eof47;
case 47:
    goto st0;
tr34:
/* #line 125 "nmea_parser.rl" */
    {
        /* Save the GEOID units. */
        msg->data.gaa.geoid_unit = *p;
    }
    goto st26;
st26:
    if ( ++p == pe )
        goto _test_eof26;
case 26:
/* #line 584 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto st20;
    goto st0;
tr31:
/* #line 115 "nmea_parser.rl" */
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
tr32:
/* #line 120 "nmea_parser.rl" */
    {
        /* Update GEOID. */
        nmea_parser_set_value(&msg->data.gaa.geoid_sep, &index, &have_dot, *p, 3);
    }
    goto st27;
st27:
    if ( ++p == pe )
        goto _test_eof27;
case 27:
/* #line 612 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st19;
        case 46: goto tr32;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr32;
    goto st0;
tr29:
/* #line 110 "nmea_parser.rl" */
    {
        /* Save the altitude units. */
        msg->data.gaa.alt_unit = *p;
    }
    goto st28;
st28:
    if ( ++p == pe )
        goto _test_eof28;
case 28:
/* #line 631 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto st18;
    goto st0;
tr23:
/* #line 95 "nmea_parser.rl" */
    {
        msg->data.gaa.used = (uint8_t)(msg->data.gaa.used * 10);
        msg->data.gaa.used = (uint8_t)(*p - '0' + msg->data.gaa.used);
    }
    goto st29;
st29:
    if ( ++p == pe )
        goto _test_eof29;
case 29:
/* #line 646 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr22;
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr41;
    goto st0;
tr41:
/* #line 95 "nmea_parser.rl" */
    {
        msg->data.gaa.used = (uint8_t)(msg->data.gaa.used * 10);
        msg->data.gaa.used = (uint8_t)(*p - '0' + msg->data.gaa.used);
    }
    goto st30;
st30:
    if ( ++p == pe )
        goto _test_eof30;
case 30:
/* #line 663 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr22;
    goto st0;
tr21:
/* #line 91 "nmea_parser.rl" */
    {
        msg->data.gaa.fix = (uint8_t)(*p - '0');
    }
    goto st31;
st31:
    if ( ++p == pe )
        goto _test_eof31;
case 31:
/* #line 677 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr20;
    goto st0;
tr19:
/* #line 79 "nmea_parser.rl" */
    {
        /* Save longitude E/W. */
        msg->longitude_ew = *p;
    }
    goto st32;
st32:
    if ( ++p == pe )
        goto _test_eof32;
case 32:
/* #line 692 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto st13;
    goto st0;
tr15:
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
/* #line 707 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr14;
    goto st0;
st34:
    if ( ++p == pe )
        goto _test_eof34;
case 34:
    if ( (*p) == 76 )
        goto tr42;
    goto st0;
tr42:
/* #line 132 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        msg->msg_id = NMEA_MSG_GLL;
    }
    goto st35;
st35:
    if ( ++p == pe )
        goto _test_eof35;
case 35:
/* #line 729 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr43;
    goto st0;
tr43:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st36;
tr45:
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
/* #line 752 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st37;
        case 46: goto tr45;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr45;
    goto st0;
st37:
    if ( ++p == pe )
        goto _test_eof37;
case 37:
    switch( (*p) ) {
        case 44: goto tr46;
        case 78: goto tr47;
        case 83: goto tr47;
    }
    goto st0;
tr46:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st38;
tr49:
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
/* #line 789 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st39;
        case 46: goto tr49;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr49;
    goto st0;
st39:
    if ( ++p == pe )
        goto _test_eof39;
case 39:
    switch( (*p) ) {
        case 44: goto tr50;
        case 69: goto tr51;
        case 87: goto tr51;
    }
    goto st0;
tr50:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    goto st40;
tr53:
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
/* #line 826 "nmea_parser.c" */
    switch( (*p) ) {
        case 44: goto st41;
        case 46: goto tr53;
    }
    if ( 48 <= (*p) && (*p) <= 57 )
        goto tr53;
    goto st0;
st41:
    if ( ++p == pe )
        goto _test_eof41;
case 41:
    if ( (*p) == 44 )
        goto st42;
    if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto tr55;
    } else if ( (*p) >= 65 )
        goto tr55;
    goto st0;
st42:
    if ( ++p == pe )
        goto _test_eof42;
case 42:
    if ( (*p) == 42 )
        goto tr36;
    if ( (*p) > 90 ) {
        if ( 97 <= (*p) && (*p) <= 122 )
            goto tr56;
    } else if ( (*p) >= 65 )
        goto tr56;
    goto st0;
tr56:
/* #line 142 "nmea_parser.rl" */
    {
        /* Save the data mode. */
        msg->data.gll.mode = *p;
    }
    goto st43;
st43:
    if ( ++p == pe )
        goto _test_eof43;
case 43:
/* #line 869 "nmea_parser.c" */
    if ( (*p) == 42 )
        goto tr36;
    goto st0;
tr55:
/* #line 137 "nmea_parser.rl" */
    {
        /* Save the data status. */
        msg->data.gll.status = *p;
    }
    goto st44;
st44:
    if ( ++p == pe )
        goto _test_eof44;
case 44:
/* #line 884 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto st42;
    goto st0;
tr51:
/* #line 79 "nmea_parser.rl" */
    {
        /* Save longitude E/W. */
        msg->longitude_ew = *p;
    }
    goto st45;
st45:
    if ( ++p == pe )
        goto _test_eof45;
case 45:
/* #line 899 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr50;
    goto st0;
tr47:
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
/* #line 914 "nmea_parser.c" */
    if ( (*p) == 44 )
        goto tr46;
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
    _test_eof47: cs = 47; goto _test_eof;
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

    _test_eof: {}
    _out: {}
    }

/* #line 292 "nmea_parser.rl" */
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
