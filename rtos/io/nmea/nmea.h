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
#define NMEA_CSUM_ERROR     (-2302)

/* NMEA message definitions/flags. */
#define NMEA_MSG_UNKNOWN    (0x0)
#define NMEA_MSG_GGA        (0x1)
#define NMEA_MSG_GLL        (0x2)
#define NMEA_MSG_GSA        (0x4)
#define NMEA_MSG_GSV        (0x8)
#define NMEA_MSG_RMC        (0x10)
#define NMEA_MSG_VTG        (0x20)

/* NMEA message definition. */
typedef struct _nmea_data
{
    /* Latitude as DDMMmmmmm. */
    uint32_t    latitude;

    /* Longitude as DDMMmmmmm. */
    uint32_t    longitude;

    /* UTC time as HHMMSSmmm. */
    uint32_t    utc;

    /* Date as DDMMYY. */
    uint32_t    date;

    /* Speed over ground in mili knots. */
    uint32_t    speed_knots;

    /* Speed over ground in meter p/h. */
    uint32_t    speed_mph;

    /* Course over ground in mili degrees. */
    uint32_t    course;

    /* HDOP in milidb. */
    uint32_t    hdop;

    /* Altitude in mili units. */
    uint32_t    altitude;

    /* GEOID separation in mili units. */
    uint32_t    geoid_sep;

    /* Latitude N/S, Longitude E/W. */
    uint8_t     latitude_ns;
    uint8_t     longitude_ew;

    /* Altitude and GEOID unit. */
    uint8_t     alt_unit;
    uint8_t     geoid_unit;
    uint8_t     geoid_neg;

    /* Fix indicator. */
    uint8_t     fix;

    /* Satellites used. */
    uint8_t     used;

    /* Data status. */
    uint8_t     status;

    /* Data mode. */
    uint8_t     mode;

    /* Structure padding. */
    uint8_t     pad[3];

} NMEA_DATA;

/* NMEA bus/device definition. */
typedef struct _nmea
{
    /* File descriptor associated with this NMEA bus/device. */
    FD          fd;

    /* NMEA data buffer. */
    NMEA_DATA   data;
} NMEA;

/* Function prototypes. */
int32_t nmea_fetch_data(NMEA *, uint8_t);
int32_t nmea_parse_message(NMEA *, uint8_t *, uint8_t *);
void nmea_parser_set_value(uint32_t *, uint8_t *, uint8_t *, uint8_t, uint8_t);

#endif /* CONFIG_NMEA */
#endif /* _NMEA_H_ */
