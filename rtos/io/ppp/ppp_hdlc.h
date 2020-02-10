/*
 * ppp_hdlc.h
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
#ifndef _PPP_HDLC_H_
#define _PPP_HDLC_H_

#include <kernel.h>

#ifdef IO_PPP

/* Status code definitions. */
#define HDLC_STREAM_ERROR       -920

/* HDLC protocol definitions. */
#define PPP_FLAG                (0x7E)
#define PPP_ESCAPE              (0x7D)

/* Function prototypes. */
int32_t ppp_hdlc_header_parse(FS_BUFFER_LIST *, uint8_t);
int32_t ppp_hdlc_unescape(FS_BUFFER_LIST *buffer);
void ppp_hdlc_unescape_one(FS_BUFFER *, uint8_t *);
int32_t ppp_hdlc_header_add(FS_BUFFER_LIST *, uint32_t *, uint8_t, uint8_t, uint8_t);
int32_t ppp_hdlc_escape(FS_BUFFER_LIST *, FS_BUFFER_LIST *, uint32_t *, uint8_t, uint8_t);

#endif /* IO_PPP */

#endif /* _PPP_HDLC_H_ */
