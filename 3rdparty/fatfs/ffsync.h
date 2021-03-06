/*
 * ffsync.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _FFSYNC_H_
#define _FFSYNC_H_

#include <kernel.h>

#ifdef CONFIG_FS
#include <fs.h>

#ifdef FS_FAT
#include <ff.h>

#if (_FS_REENTRANT == TRUE)
typedef enum
{
    FFSYNC_ERROR = 0,
    FFSYNC_SUCCESS = 1,
} FFSYNC_STATUS;

/* Function prototypes. */
int ff_cre_syncobj (BYTE vol, _SYNC_t *sobj);
int ff_req_grant (_SYNC_t *sobj);
void ff_rel_grant (_SYNC_t *sobj);
int ff_del_syncobj (_SYNC_t *sobj);

#endif /* (_FS_REENTRANT == TRUE) */
#endif /* FS_FAT */
#endif /* CONFIG_FS */
#endif /* _FFSYNC_H_ */
