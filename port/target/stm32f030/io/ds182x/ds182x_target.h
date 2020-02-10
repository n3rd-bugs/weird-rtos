/*
 * ds182x_target.h
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _DS182X_TARGET_H_
#define _DS182X_TARGET_H_
#include <kernel.h>

#ifdef GPIO_DS182X
#include <ds182x_stm32.h>

/* Export default DS182X bus. */
#define DS182X_DEFAULT          (&ds182x_stm32.bus)

/* Hook-up DS182X driver. */
#define DS182X_TGT_INIT()       ds182x_stm32_init()

#endif /* GPIO_DS182X */
#endif /* _DS182X_TARGET_H_ */
