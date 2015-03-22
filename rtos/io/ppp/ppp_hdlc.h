/*
 * ppp_hdlc.h
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
#ifndef _PPP_HDLC_H_
#define _PPP_HDLC_H_

#include <os.h>

#ifdef CONFIG_PPP

/* Status code definitions. */
#define HDLC_STREAM_ERROR       -920

/* HDLC protocol definitions. */
#define PPP_FLAG                (0x7E)
#define PPP_ESCAPE              (0x7D)

/* Function prototypes. */
int32_t ppp_hdlc_header_parse(FS_BUFFER *, uint8_t);
int32_t ppp_hdlc_unescape(FS_BUFFER *buffer);
void ppp_hdlc_unescape_one(FS_BUFFER_ONE *, uint8_t *);
int32_t ppp_hdlc_header_add(FS_BUFFER *, uint32_t *, uint8_t, uint8_t);
int32_t ppp_hdlc_escape(FS_BUFFER *, FS_BUFFER *, uint32_t *, uint8_t);

#endif /* CONFIG_PPP */

#endif /* _PPP_HDLC_H_ */
