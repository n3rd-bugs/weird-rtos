/*
 * fs_avr.h
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
#ifndef _FS_AVR_H_
#define _FS_AVR_H_
#include <kernel.h>

#ifdef CONFIG_FS
#if (defined(FS_FAT) && defined(CONFIG_MMC))
#include <fs_avr_config.h>
#endif /* (defined(FS_FAT) && defined(CONFIG_MMC)) */

/* Function prototypes. */
void fs_avr_init(void);

#endif /* CONFIG_FS */
#endif /* _FS_AVR_H_ */
