/*
 * nmea_p.h
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
#ifndef _NMEA_P_H_
#define _NMEA_P_H_
#include <kernel.h>

#ifdef IO_NMEA
#include <nmea.h>

/* Function prototypes. */
void nmea_ublox_configure(NMEA *, uint8_t);
void nmea_ublox_set_msg_rate(NMEA *, uint8_t *, uint8_t);

#endif /* IO_NMEA */
#endif /* _NMEA_P_H_ */
