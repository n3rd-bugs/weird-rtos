/*
 * fs_target.h
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
#ifndef _FS_TARGET_H_
#define _FS_TARGET_H_
#include <kernel.h>

#ifdef CONFIG_FS
#include <fs_stm32f103.h>

/* Hook-up FS OS stack. */
#define FS_TGT_INIT()           fs_stm32_init()

#endif /* CONFIG_FS */
#endif /* _FS_TARGET_H_ */
