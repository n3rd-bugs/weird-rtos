/*
 * nmea_p.c
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
#include <nmea_p.h>
#include <rtl.h>

/*
 * nmea_ublox_configure
 * @nmea: NMEA instance.
 * @msg_cntrl: Bit field to specify the messages to be received from the device.
 * This will configure a u-blox device to only send the requested messages.
 */
void nmea_ublox_configure(NMEA *nmea, uint8_t msg_cntrl)
{
    /* Configure the required messages. */
    nmea_ublox_set_msg_rate(nmea, (uint8_t *)"GGA", ((msg_cntrl & NMEA_MSG_GGA) != 0));
    nmea_ublox_set_msg_rate(nmea, (uint8_t *)"GLL", ((msg_cntrl & NMEA_MSG_GLL) != 0));
    nmea_ublox_set_msg_rate(nmea, (uint8_t *)"GSA", ((msg_cntrl & NMEA_MSG_GSA) != 0));
    nmea_ublox_set_msg_rate(nmea, (uint8_t *)"GSV", ((msg_cntrl & NMEA_MSG_GSV) != 0));
    nmea_ublox_set_msg_rate(nmea, (uint8_t *)"RMC", ((msg_cntrl & NMEA_MSG_RMC) != 0));
    nmea_ublox_set_msg_rate(nmea, (uint8_t *)"VTG", ((msg_cntrl & NMEA_MSG_VTG) != 0));

} /* nmea_ublox_configure */

/*
 * nmea_ublox_set_msg_rate
 * @nmea: NMEA instance.
 * @msg: Message for which rate is needed to be set.
 * @rate: Rate needed to be set.
 * This will set rate for the given message on UART for a u-blox device.
 */
void nmea_ublox_set_msg_rate(NMEA *nmea, uint8_t *msg, uint8_t rate)
{
    uint8_t csum_tx = 0^'P'^'U'^'B'^'X'^','^'4'^'0'^','^','^','^'0'^','^'0'^','^'0'^','^'0'^','^'0';
    uint8_t buffer[5];

    /* Flush any RX data on this descriptor. */
    fs_flush_rx(nmea->fd);

    /* Send start of the message. */
    fs_puts(nmea->fd, (const uint8_t *)"$PUBX,40,", -1);

    /* Set message ID. */
    csum_tx ^= msg[0] ^ msg[1] ^ msg[2];
    fs_puts(nmea->fd, msg, 3);
    fs_puts(nmea->fd, (const uint8_t *)",0,", -1);

    /* Set rate. */
    rtl_ultoa(rate, buffer, 1, RTL_ULTOA_LEADING_ZEROS);
    csum_tx ^= buffer[0];
    fs_puts(nmea->fd, buffer, 1);
    fs_puts(nmea->fd, (const uint8_t *)",0,0,0,0*", -1);

    /* Populate csum. */
    buffer[0] = (uint8_t)(((csum_tx & 0xF0) >> 4) + ((((csum_tx & 0xF0) >> 4) > 9) ? ('A' - 10) : ('0')));
    buffer[1] = (uint8_t)((csum_tx & 0xF) + (((csum_tx & 0xF) > 9) ? ('A' - 10) : ('0')));
    fs_puts(nmea->fd, buffer, 2);
    fs_puts(nmea->fd, (const uint8_t *)"\r\n", -1);

} /* nmea_ublox_set_msg_rate */
#endif /* IO_NMEA */
