/*
 * ppp_packet.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _PPP_PACKET_H_
#define _PPP_PACKET_H_

#include <kernel.h>

#ifdef CONFIG_PPP

/* Maximum option value length for an option in PPP packet. */
#ifndef CMAKE_BUILD
#define PPP_MAX_OPTION_SIZE     16
#endif /* CMAKE_BUILD */

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
int32_t ppp_packet_protocol_parse(FS_BUFFER *, uint16_t *, uint8_t);
int32_t ppp_packet_protocol_add(FS_BUFFER *, uint16_t, uint8_t, uint8_t);
int32_t ppp_packet_configuration_header_parse(FS_BUFFER *, PPP_CONF_PKT *);
int32_t ppp_packet_configuration_option_parse(FS_BUFFER *, PPP_CONF_OPT *);
int32_t ppp_packet_configuration_header_add(FS_BUFFER *, PPP_CONF_PKT *);
int32_t ppp_packet_configuration_option_add(FS_BUFFER *, PPP_CONF_OPT *);

#endif /* CONFIG_PPP */

#endif /* _PPP_PACKET_H_ */
