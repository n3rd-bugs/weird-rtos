/*
 * ppp_packet.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _PPP_PACKET_H_
#define _PPP_PACKET_H_

#include <os.h>

#ifdef CONFIG_PPP

/* PPP option structure. */
typedef struct _ppp_pkt_opt
{
    uint8_t     *data;      /* Option data. */
    uint8_t     type;       /* Option type. */
    uint8_t     length;     /* Option length. */
    uint8_t     pad[2];     /* Option structure padding. */

} PPP_PKT_OPT ;

/* PPP packet data. */
typedef struct _ppp_pkt
{
    /* PPP packet data. */
    uint16_t    length;     /* PPP packet length (not including protocol). */
    uint8_t     code;       /* PPP packet code. */
    uint8_t     id;         /* PPP packet id. */

} PPP_PKT;

/* Function prototypes. */
int32_t ppp_packet_protocol_parse(FS_BUFFER *, uint16_t *, uint8_t);
int32_t ppp_packet_protocol_add(FS_BUFFER *, uint16_t, uint8_t);
int32_t ppp_packet_configuration_header_parse(PPP_PKT *, FS_BUFFER *);
int32_t ppp_packet_configuration_option_parse(PPP_PKT_OPT *, FS_BUFFER *);
int32_t ppp_packet_configuration_header_add(PPP_PKT *, FS_BUFFER *);
int32_t ppp_packet_configuration_option_add(PPP_PKT_OPT *, FS_BUFFER *);

#endif /* CONFIG_PPP */

#endif /* _PPP_PACKET_H_ */
