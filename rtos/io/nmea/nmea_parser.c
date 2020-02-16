
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

#ifdef IO_NMEA
#include <nmea.h>
#include <fs.h>
#include <fs_buffer.h>


/* #line 269 "nmea_parser.rl" */


/* Machine definitions. */

/* #line 32 "nmea_parser.c" */
static const unsigned char _nmea_key_offsets[] = {
    0, 0, 12, 14, 16, 19, 22, 23,
    24, 28, 32, 35, 39, 42, 45, 48,
    52, 56, 61, 66, 71, 75, 79, 83,
    87, 88, 89, 90, 94, 95, 98, 99,
    100, 101, 102, 103, 104, 108, 111, 115,
    118, 122, 127, 132, 133, 134, 135, 136,
    138, 140, 149, 150, 151, 152, 156, 161,
    165, 168, 172, 175, 179, 183, 186, 190,
    194, 195, 196, 197, 198, 199, 200, 204,
    206, 210, 212, 216, 218, 222, 224, 225,
    226, 227
};

static const char _nmea_trans_keys[] = {
    10, 13, 36, 42, 44, 46, 48, 57,
    65, 90, 97, 122, 65, 90, 65, 90,
    71, 82, 86, 71, 76, 83, 65, 44,
    44, 46, 48, 57, 44, 46, 48, 57,
    44, 78, 83, 44, 46, 48, 57, 44,
    69, 87, 44, 48, 57, 44, 48, 57,
    44, 46, 48, 57, 44, 46, 48, 57,
    44, 65, 90, 97, 122, 44, 45, 46,
    48, 57, 44, 65, 90, 97, 122, 44,
    46, 48, 57, 42, 46, 48, 57, 48,
    57, 65, 70, 48, 57, 65, 70, 13,
    10, 44, 44, 46, 48, 57, 44, 44,
    48, 57, 44, 44, 44, 44, 76, 44,
    44, 46, 48, 57, 44, 78, 83, 44,
    46, 48, 57, 44, 69, 87, 44, 46,
    48, 57, 44, 65, 90, 97, 122, 42,
    65, 90, 97, 122, 42, 44, 44, 44,
    65, 86, 42, 44, 42, 44, 46, 48,
    57, 65, 90, 97, 122, 77, 67, 44,
    44, 46, 48, 57, 44, 65, 90, 97,
    122, 44, 46, 48, 57, 44, 78, 83,
    44, 46, 48, 57, 44, 69, 87, 44,
    46, 48, 57, 44, 46, 48, 57, 44,
    48, 57, 44, 46, 48, 57, 44, 46,
    48, 57, 44, 44, 44, 84, 71, 44,
    44, 46, 48, 57, 44, 84, 44, 46,
    48, 57, 44, 77, 44, 46, 48, 57,
    44, 78, 44, 46, 48, 57, 44, 75,
    44, 44, 44, 0
};

static const char _nmea_single_lengths[] = {
    0, 6, 0, 0, 3, 3, 1, 1,
    2, 2, 3, 2, 3, 1, 1, 2,
    2, 1, 3, 1, 2, 2, 0, 0,
    1, 1, 1, 2, 1, 1, 1, 1,
    1, 1, 1, 1, 2, 3, 2, 3,
    2, 1, 1, 1, 1, 1, 1, 2,
    2, 3, 1, 1, 1, 2, 1, 2,
    3, 2, 3, 2, 2, 1, 2, 2,
    1, 1, 1, 1, 1, 1, 2, 2,
    2, 2, 2, 2, 2, 2, 1, 1,
    1, 0
};

static const char _nmea_range_lengths[] = {
    0, 3, 1, 1, 0, 0, 0, 0,
    1, 1, 0, 1, 0, 1, 1, 1,
    1, 2, 1, 2, 1, 1, 2, 2,
    0, 0, 0, 1, 0, 1, 0, 0,
    0, 0, 0, 0, 1, 0, 1, 0,
    1, 2, 2, 0, 0, 0, 0, 0,
    0, 3, 0, 0, 0, 1, 2, 1,
    0, 1, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 1, 0,
    1, 0, 1, 0, 1, 0, 0, 0,
    0, 0
};

static const short _nmea_index_offsets[] = {
    0, 0, 10, 12, 14, 18, 22, 24,
    26, 30, 34, 38, 42, 46, 49, 52,
    56, 60, 64, 69, 73, 77, 81, 84,
    87, 89, 91, 93, 97, 99, 102, 104,
    106, 108, 110, 112, 114, 118, 122, 126,
    130, 134, 138, 142, 144, 146, 148, 150,
    153, 156, 163, 165, 167, 169, 173, 177,
    181, 185, 189, 193, 197, 201, 204, 208,
    212, 214, 216, 218, 220, 222, 224, 228,
    231, 235, 238, 242, 245, 249, 252, 254,
    256, 258
};

static const char _nmea_indicies[] = {
    0, 0, 2, 0, 0, 0, 0, 0,
    0, 1, 3, 1, 4, 1, 5, 6,
    7, 1, 8, 9, 10, 1, 11, 1,
    12, 1, 13, 14, 14, 1, 15, 16,
    16, 1, 17, 18, 18, 1, 19, 20,
    20, 1, 21, 22, 22, 1, 23, 24,
    1, 25, 26, 1, 27, 28, 28, 1,
    29, 30, 30, 1, 31, 32, 32, 1,
    33, 34, 35, 35, 1, 36, 37, 37,
    1, 38, 36, 36, 1, 39, 38, 38,
    1, 40, 40, 1, 41, 41, 1, 42,
    1, 43, 1, 36, 1, 33, 35, 35,
    1, 31, 1, 25, 44, 1, 25, 1,
    23, 1, 21, 1, 17, 1, 45, 1,
    46, 1, 47, 48, 48, 1, 49, 50,
    50, 1, 51, 52, 52, 1, 53, 54,
    54, 1, 55, 56, 56, 1, 57, 58,
    58, 1, 39, 59, 59, 1, 39, 1,
    57, 1, 53, 1, 49, 1, 60, 61,
    1, 39, 62, 1, 39, 62, 62, 62,
    62, 62, 1, 63, 1, 64, 1, 65,
    1, 66, 67, 67, 1, 68, 69, 69,
    1, 70, 71, 71, 1, 72, 73, 73,
    1, 74, 75, 75, 1, 76, 77, 77,
    1, 78, 79, 79, 1, 80, 81, 81,
    1, 82, 83, 1, 84, 82, 82, 1,
    57, 84, 84, 1, 76, 1, 72, 1,
    68, 1, 85, 1, 86, 1, 87, 1,
    88, 89, 89, 1, 90, 91, 1, 92,
    90, 90, 1, 93, 94, 1, 95, 96,
    96, 1, 97, 98, 1, 99, 100, 100,
    1, 57, 101, 1, 97, 1, 93, 1,
    90, 1, 1, 0
};

static const char _nmea_trans_targs[] = {
    1, 0, 2, 3, 4, 5, 50, 67,
    6, 34, 47, 7, 8, 9, 8, 10,
    9, 11, 33, 12, 11, 13, 32, 14,
    31, 15, 29, 16, 15, 17, 16, 18,
    28, 19, 27, 27, 20, 26, 21, 22,
    23, 24, 25, 81, 30, 35, 36, 37,
    36, 38, 46, 39, 38, 40, 45, 41,
    40, 42, 44, 43, 48, 48, 49, 51,
    52, 53, 54, 53, 55, 66, 56, 55,
    57, 65, 58, 57, 59, 64, 60, 59,
    61, 60, 62, 61, 63, 68, 69, 70,
    71, 70, 72, 80, 73, 74, 79, 75,
    74, 76, 78, 77, 76, 44
};

static const char _nmea_trans_actions[] = {
    0, 0, 1, 2, 2, 0, 0, 0,
    0, 0, 0, 3, 4, 4, 5, 0,
    6, 4, 7, 0, 8, 0, 9, 4,
    10, 4, 11, 4, 12, 0, 13, 0,
    14, 0, 15, 16, 0, 17, 0, 18,
    19, 20, 0, 0, 11, 21, 4, 0,
    6, 4, 7, 0, 8, 4, 9, 0,
    5, 0, 22, 23, 24, 25, 0, 0,
    26, 4, 0, 5, 4, 22, 0, 6,
    4, 7, 0, 8, 4, 9, 4, 27,
    4, 28, 0, 29, 0, 0, 30, 4,
    0, 28, 0, 0, 0, 4, 0, 0,
    27, 4, 0, 0, 31, 0
};

static const int nmea_start = 1;
static const int nmea_first_final = 81;
static const int nmea_error = 0;

static const int nmea_en_main = 1;


/* #line 273 "nmea_parser.rl" */

/*
 * nmea_parse_message
 * @nmea: NMEA instance.
 * @talker_id: Talker ID will be returned here.
 * @msg_id: Received message ID will be returned here.
 * @return: Success will be returned if a message was successfully parsed,
 *  NMEA_READ_ERROR will be returned if an error occurred while reading from
 *      file descriptor,
 *  NMEA_SEQUENCE_ERROR will be returned if an invalid sequence was detected,
 *  NMEA_CSUM_ERROR will be returned if checksum was not valid.
 * This function will return a parsed reading from a NMEA bus/device.
 */
int32_t nmea_parse_message(NMEA *nmea, uint8_t *talker_id, uint8_t *msg_id)
{
    int32_t status = SUCCESS;
    uint8_t chr[2], index, have_dot;
    uint8_t *p = &chr[0];
    uint8_t *pe = &chr[1];
    uint8_t csum = 0;
    uint8_t csum_got = 0;
    uint8_t csum_computed = 0;
    char cs = nmea_start;

    /* Remove some compiler warning. */
    UNUSED_PARAM(nmea_en_main);

    for (;;)
    {
        /* Read a byte from the file descriptor. */
        status = fs_gets(nmea->fd, chr, 1);

        /* If we did not read expected number of bytes. */
        if ((status > 0) && (status != 1))
        {
            /* Return error to the caller. */
            status = NMEA_READ_ERROR;
        }

        /* If we did not read expected data. */
        if (status != 1)
        {
            break;
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

/* #line 256 "nmea_parser.c" */
    {
    int _klen;
    const char *_keys;
    int _trans;

    if ( p == pe )
        goto _test_eof;
    if ( cs == 0 )
        goto _out;
_resume:
    _keys = _nmea_trans_keys + _nmea_key_offsets[cs];
    _trans = _nmea_index_offsets[cs];

    _klen = _nmea_single_lengths[cs];
    if ( _klen > 0 ) {
        const char *_lower = _keys;
        const char *_mid;
        const char *_upper = _keys + _klen - 1;
        while (1) {
            if ( _upper < _lower )
                break;

            _mid = _lower + ((_upper-_lower) >> 1);
            if ( (*p) < *_mid )
                _upper = _mid - 1;
            else if ( (*p) > *_mid )
                _lower = _mid + 1;
            else {
                _trans += (unsigned int)(_mid - _keys);
                goto _match;
            }
        }
        _keys += _klen;
        _trans += _klen;
    }

    _klen = _nmea_range_lengths[cs];
    if ( _klen > 0 ) {
        const char *_lower = _keys;
        const char *_mid;
        const char *_upper = _keys + (_klen<<1) - 2;
        while (1) {
            if ( _upper < _lower )
                break;

            _mid = _lower + (((_upper-_lower) >> 1) & ~1);
            if ( (*p) < _mid[0] )
                _upper = _mid - 2;
            else if ( (*p) > _mid[1] )
                _lower = _mid + 2;
            else {
                _trans += (unsigned int)((_mid - _keys)>>1);
                goto _match;
            }
        }
        _trans += _klen;
    }

_match:
    _trans = _nmea_indicies[_trans];
    cs = _nmea_trans_targs[_trans];

    if ( _nmea_trans_actions[_trans] == 0 )
        goto _again;

    switch ( _nmea_trans_actions[_trans] ) {
    case 20:
/* #line 33 "nmea_parser.rl" */
    {
        /* Save message checksum. */
        csum_got = (uint8_t)(csum_got << (4 * index));
        csum_got |= (uint8_t)(*p - ((*p > '9') ? ('A' - 10) : '0'));
        index ++;
    }
    break;
    case 18:
/* #line 40 "nmea_parser.rl" */
    {
        /* Save the computed checksum. */
        csum_computed = csum;
    }
    break;
    case 4:
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    break;
    case 2:
/* #line 51 "nmea_parser.rl" */
    {
        /* Save the talker ID. */
        if (talker_id)
        {
            talker_id[index != 0] = *p;
        }
        index++;
    }
    break;
    case 5:
/* #line 62 "nmea_parser.rl" */
    {
        /* Update UTC */
        nmea_parser_set_value(&nmea->data.utc, &index, &have_dot, *p, 3);
    }
    break;
    case 6:
/* #line 67 "nmea_parser.rl" */
    {
        /* Update Latitude */
        nmea_parser_set_value(&nmea->data.latitude, &index, &have_dot, *p, 5);
    }
    break;
    case 7:
/* #line 72 "nmea_parser.rl" */
    {
        /* Save latitude N/S. */
        nmea->data.latitude_ns = *p;
    }
    break;
    case 8:
/* #line 77 "nmea_parser.rl" */
    {
        /* Update Longitude */
        nmea_parser_set_value(&nmea->data.longitude, &index, &have_dot, *p, 5);
    }
    break;
    case 27:
/* #line 82 "nmea_parser.rl" */
    {
        /* Update speed knots */
        nmea_parser_set_value(&nmea->data.speed_knots, &index, &have_dot, *p, 3);
    }
    break;
    case 31:
/* #line 87 "nmea_parser.rl" */
    {
        /* Update speed meter p/h */
        nmea_parser_set_value(&nmea->data.speed_mph, &index, &have_dot, *p, 3);
    }
    break;
    case 28:
/* #line 92 "nmea_parser.rl" */
    {
        /* Update course */
        nmea_parser_set_value(&nmea->data.course, &index, &have_dot, *p, 3);
    }
    break;
    case 29:
/* #line 97 "nmea_parser.rl" */
    {
        /* Update date */
        nmea_parser_set_value(&nmea->data.date, &index, &have_dot, *p, 0);
    }
    break;
    case 9:
/* #line 102 "nmea_parser.rl" */
    {
        /* Save longitude E/W. */
        nmea->data.longitude_ew = *p;
    }
    break;
    case 22:
/* #line 107 "nmea_parser.rl" */
    {
        /* Save the data status. */
        nmea->data.status = *p;
    }
    break;
    case 23:
/* #line 112 "nmea_parser.rl" */
    {
        /* Save the data mode. */
        nmea->data.mode = *p;
    }
    break;
    case 10:
/* #line 117 "nmea_parser.rl" */
    {
        nmea->data.fix = (uint8_t)(*p - '0');
    }
    break;
    case 11:
/* #line 121 "nmea_parser.rl" */
    {
        nmea->data.used = (uint8_t)(nmea->data.used * 10);
        nmea->data.used = (uint8_t)(*p - '0' + nmea->data.used);
    }
    break;
    case 12:
/* #line 126 "nmea_parser.rl" */
    {
        /* Update HDOP. */
        nmea_parser_set_value(&nmea->data.hdop, &index, &have_dot, *p, 3);
    }
    break;
    case 13:
/* #line 131 "nmea_parser.rl" */
    {
        /* Update altitude. */
        nmea_parser_set_value(&nmea->data.altitude, &index, &have_dot, *p, 3);
    }
    break;
    case 14:
/* #line 136 "nmea_parser.rl" */
    {
        /* Save the altitude units. */
        nmea->data.alt_unit = *p;
    }
    break;
    case 16:
/* #line 146 "nmea_parser.rl" */
    {
        /* Update GEOID. */
        nmea_parser_set_value(&nmea->data.geoid_sep, &index, &have_dot, *p, 3);
    }
    break;
    case 17:
/* #line 151 "nmea_parser.rl" */
    {
        /* Save the GEOID units. */
        nmea->data.geoid_unit = *p;
    }
    break;
    case 3:
/* #line 158 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        *msg_id = NMEA_MSG_GGA;
    }
    break;
    case 21:
/* #line 165 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        *msg_id = NMEA_MSG_GLL;
    }
    break;
    case 26:
/* #line 172 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        *msg_id = NMEA_MSG_RMC;
    }
    break;
    case 30:
/* #line 179 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        *msg_id = NMEA_MSG_VTG;
    }
    break;
    case 24:
/* #line 186 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        *msg_id = NMEA_MSG_GSA;
    }
    break;
    case 25:
/* #line 193 "nmea_parser.rl" */
    {
        /* Save the message ID. */
        *msg_id = NMEA_MSG_GSV;
    }
    break;
    case 1:
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
    break;
    case 19:
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
    break;
    case 15:
/* #line 141 "nmea_parser.rl" */
    {
        /* Set the GEOID as negative. */
        nmea->data.geoid_neg = TRUE;
    }
/* #line 45 "nmea_parser.rl" */
    {
        /* Reset index. */
        index = 0;
        have_dot = FALSE;
    }
    break;
/* #line 566 "nmea_parser.c" */
    }

_again:
    if ( cs == 0 )
        goto _out;
    if ( ++p != pe )
        goto _resume;
    _test_eof: {}
    _out: {}
    }

/* #line 332 "nmea_parser.rl" */
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

    /* Return status to the caller. */
    return (status);

} /* nmea_parse_message */

#endif /* IO_NMEA */
