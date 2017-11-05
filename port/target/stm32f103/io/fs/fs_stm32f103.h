/*
 * fs_stm32f103.h
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
#ifndef _FS_STM32F103_H_
#define _FS_STM32F103_H_
#include <kernel.h>

#ifdef CONFIG_FS

/* Function prototypes. */
void fs_stm32_init(void);

#endif /* CONFIG_FS */
#endif /* _FS_STM32F103_H_ */
