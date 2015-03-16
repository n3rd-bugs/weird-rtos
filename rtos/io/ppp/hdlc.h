/*
 * hdlc.h
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
#ifndef _HDLC_H_
#define _HDLC_H_

#include <os.h>

#ifdef PPP_HDLC

/* Status code definitions. */
#define HDLC_STREAM_ERROR       -1100

/* HDLC protocol definitions. */
#define HDLC_ESCAPE             (0x7D)

/* Function prototypes. */
int32_t hdlc_header_parse(FS_BUFFER *, uint8_t);
int32_t hdlc_header_add(FS_BUFFER *, uint32_t *, uint8_t);
int32_t hdlc_escape(FS_BUFFER *, uint32_t *);
int32_t hdlc_unescape(FS_BUFFER *buffer);

#endif /* PPP_HDLC */

#endif /* _HDLC_H_ */
