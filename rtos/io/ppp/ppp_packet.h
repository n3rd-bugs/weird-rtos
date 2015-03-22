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

/* Maximum option value length for an option in PPP packet. */
#define PPP_MAX_OPTION_SIZE     16

/* PPP configuration option structure. */
typedef struct _ppp_conf_opt
{
    uint8_t     type;       /* Option type. */
    uint8_t     length;     /* Option length. */
    uint8_t     data[PPP_MAX_OPTION_SIZE];  /* Option data. */
    uint8_t     pad[2];     /* Option structure padding. */

} PPP_CONF_OPT;

/* PPP configuration packet data. */
typedef struct _ppp_conf_pkt
{
    /* PPP packet data. */
    uint16_t    length;     /* PPP packet length (not including protocol). */
    uint8_t     code;       /* PPP packet code. */
    uint8_t     id;         /* PPP packet id. */

} PPP_CONF_PKT;

/* Function prototypes. */
int32_t ppp_packet_protocol_parse(FS_BUFFER_CHAIN *, uint16_t *, uint8_t);
int32_t ppp_packet_protocol_add(FS_BUFFER_CHAIN *, uint16_t, uint8_t);
int32_t ppp_packet_configuration_header_parse(FS_BUFFER_CHAIN *, PPP_CONF_PKT *);
int32_t ppp_packet_configuration_option_parse(FS_BUFFER_CHAIN *, PPP_CONF_OPT *);
int32_t ppp_packet_configuration_header_add(FS_BUFFER_CHAIN *, PPP_CONF_PKT *);
int32_t ppp_packet_configuration_option_add(FS_BUFFER_CHAIN *, PPP_CONF_OPT *);

#endif /* CONFIG_PPP */

#endif /* _PPP_PACKET_H_ */
