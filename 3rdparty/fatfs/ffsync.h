/*
 * ffsync.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _FFSYNC_H_
#define _FFSYNC_H_

#include <os.h>

#ifdef CONFIG_FS
#include <fs_avr.h>

#ifdef CONFIG_MMC
#include <ff.h>

typedef enum
{
    FFSYNC_ERROR = 0,
    FFSYNC_SUCCESS = 1,
} FFSYNC_STATUS;

/* Function prototypes. */
int ff_cre_syncobj (BYTE vol, _SYNC_t* sobj);
int ff_req_grant (_SYNC_t sobj);
void ff_rel_grant (_SYNC_t sobj);
int ff_del_syncobj (_SYNC_t sobj);

#endif /* CONFIG_MMC */
#endif /* CONFIG_FS */
#endif /* _FFSYNC_H_ */
