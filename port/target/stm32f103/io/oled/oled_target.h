/*
 * oled_target.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _OLED_TARGET_H_
#define _OLED_TARGET_H_
#include <kernel.h>

#ifdef CONFIG_OLED
#include <oled_stm32.h>

/* Hook-up OLED OS stack. */
#define OLED_TGT_INIT()             oled_stm32_init()

#endif /* CONFIG_OLED */
#endif /* _OLED_TARGET_H_ */
