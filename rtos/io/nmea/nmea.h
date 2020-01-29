/*
 * nmea.h
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
#ifndef _NMEA_H_
#define _NMEA_H_
#include <kernel.h>

#ifdef CONFIG_NMEA
#include <fs.h>

/* Error code definitions. */
#define NMEA_SEQUENCE_ERROR (-2300)
#define NMEA_READ_ERROR     (-2301)

/* NMEA message definitions. */
#define NMEA_MSG_GGA        (0)
#define NMEA_MSG_GLL        (1)
#define NMEA_MSG_GSA        (2)
#define NMEA_MSG_GSV        (3)
#define NMEA_MSG_RMC        (4)
#define NMEA_MSG_VTG        (5)
#define NMEA_MSG_UNKNOWN    (255)

/* NMEA message definition. */
typedef struct _nmea_msg
{
    /* Parsed message data. */
    union
    {
        /* Data for GGA message. */
        struct _nmea_gga
        {
            /* UTC time as HHMMSSmmm. */
            uint32_t    utc;

            /* Latitude as DDMMmmmmm. */
            uint32_t    latitude;

            /* Longitude as DDMMmmmmm. */
            uint32_t    longitude;

            /* HDOP in milidb. */
            uint32_t    hdop;

            /* Altitude in mili units. */
            uint32_t    altitude;

            /* GEOID separation in mili units. */
            uint32_t    geoid_sep;

            /* Latitude N/S, Longitude E/W. */
            uint8_t     latitude_ns;
            uint8_t     longitude_ew;

            /* Fix indicator. */
            uint8_t     fix;

            /* Satellites used. */
            uint8_t     used;

            /* Altitude and GEOID unit. */
            uint8_t     alt_unit;
            uint8_t     geoid_unit;
            uint8_t     geoid_neg;

            /* Structure padding. */
            uint8_t     pad[1];

        } gaa;

    } data;

    /* Talker ID. */
    uint8_t talker_id[2];

    /* Message ID. */
    uint8_t msg_id;

    /* Structure padding. */
    uint8_t pad[1];

} NMEA_MSG;

/* NMEA bus/device definition. */
typedef struct _nmea
{
    /* File descriptor associated with this NMEA bus/device. */
    FD  fd;
} NMEA;

/* Function prototypes. */
int32_t nmea_parse_message(NMEA *, NMEA_MSG *);
void nmea_parser_set_value(uint32_t *, uint8_t *, uint8_t *, uint8_t, uint8_t);

#endif /* CONFIG_NMEA */
#endif /* _NMEA_H_ */
