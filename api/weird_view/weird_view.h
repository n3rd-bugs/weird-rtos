/*
 * weird_view.h
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
#ifndef _WEIRD_VIEW_H_
#define _WEIRD_VIEW_H_
#include <os.h>

/* Error code definitions. */
#define WV_UNKNOWN_CMD          -20000
#define WV_INAVLID_HRD          -20001
#define WV_NO_DATA              -20002

/* Weird view protocol definitions. */
#define WV_DISC                 0x8737808B
#define WV_DISC_REPLY           0x134E3A9D
#define WV_LIST                 0x7DE0B79C
#define WV_LIST_REPLY           0x57186F8A
#define WV_REQ                  0x13AC8F62
#define WV_REQ_REPLY            0x0F4E180A

/* Weird view plugin definitions. */
#define WV_PLUGIN_LOG           0x00

/* Weird view log plugin definitions. */
#define WV_PLUGIN_LOG_UPDATE    0x00
#define WV_PLUGIN_LOG_APPEND    0x01

#endif /* _WEIRD_VIEW_SERVER_H_ */
