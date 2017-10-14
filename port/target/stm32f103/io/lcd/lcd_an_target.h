/*
 * lcd_an_target.h
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
#ifndef _LCD_AN_TARGET_H_
#define _LCD_AN_TARGET_H_
#include <kernel.h>

#ifdef CONFIG_LCD_AN
#include <lcd_an_stm32.h>

/* Hook-up LCD OS stack. */
#define LCD_AN_TGT_INIT()           lcd_an_stm32_init()

#endif /* CONFIG_LCD_AN */
#endif /* _LCD_AN_TARGET_H_ */
