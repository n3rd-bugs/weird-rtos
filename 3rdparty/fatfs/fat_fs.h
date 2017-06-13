/*
 * fat_fs.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from it's direct
 * or indirect use.
 */
#ifndef _FAT_FS_H_
#define _FAT_FS_H_

#include <os.h>

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
void fatfs_init();

#endif /* FS_FAT */
#endif /* CONFIG_FS */
#endif /* _FAT_FS_H_ */
