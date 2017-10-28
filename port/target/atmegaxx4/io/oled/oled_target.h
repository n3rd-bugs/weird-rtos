/*
 * oled_target.h
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
#ifndef _OLED_TARGET_H_
#define _OLED_TARGET_H_
#include <kernel.h>

#ifdef CONFIG_OLED
#include <oled_avr.h>

/* Hook-up OLED OS stack. */
#define OLED_TGT_INIT()             oled_avr_init()

#endif /* CONFIG_OLED */
#endif /* _OLED_TARGET_H_ */
