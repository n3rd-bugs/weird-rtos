/*
 * weird_view.h
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
#ifndef _WEIRD_VIEW_H_
#define _WEIRD_VIEW_H_
#include <kernel.h>

/* Error code definitions. */
#define WV_UNKNOWN_CMD          -20000
#define WV_INAVLID_HRD          -20001
#define WV_NO_DATA              -20002

/* Weird view protocol definitions. */
#define WV_DISC                 0x8737808B
#define WV_DISC_REPLY           0x134E3A9D
#define WV_LIST                 0x7DE0B79C
#define WV_LIST_REPLY           0x57186F8A
#define WV_UPDATE               0x13AC8F62
#define WV_UPDATE_REPLY         0x0F4E180A
#define WV_REQ                  0x81c4b463

/* Weird view plugin definitions. */
#define WV_PLUGIN_LOG           0x0
#define WV_PLUGIN_SWITCH        0x1
#define WV_PLUGIN_ANALOG        0x2
#define WV_PLUGIN_WAVE          0x3

/* Weird view log plugin definitions. */
#define WV_PLUGIN_LOG_UPDATE    0x0
#define WV_PLUGIN_LOG_APPEND    0x1

/* Weird view switch plugin definitions. */
#define WV_PLUGIN_SWITCH_OFF    0x0
#define WV_PLUGIN_SWITCH_ON     0x1

#endif /* _WEIRD_VIEW_SERVER_H_ */
