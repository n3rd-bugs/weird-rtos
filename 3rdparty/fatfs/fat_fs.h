/*
 * fat_fs.h
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
#ifndef _FAT_FS_H_
#define _FAT_FS_H_

#include <kernel.h>

#ifdef CONFIG_FS
#include <fs.h>

#ifdef FS_FAT
#include <ff.h>

/* Various configuration options. */
#define FAT_NUM_FILES   2

/* FAT file definitions. */
#define FAT_FILE_OPEN   (0x01)

/* FAT file structure. */
typedef struct _fat_file
{
    /* File system object for this file. */
    FS          fs;

    /* FAT file descriptor. */
    FIL         file;

    /* Flags to specify file state. */
    uint32_t    flags;

} FAT_FILE;

/* Function prototypes. */
void fatfs_init(void);

#endif /* FS_FAT */
#endif /* CONFIG_FS */
#endif /* _FAT_FS_H_ */
