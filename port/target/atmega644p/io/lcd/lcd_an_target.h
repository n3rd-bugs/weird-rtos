/*
 * lcd_an_target.h
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */

#ifndef _LCD_AN_TARGET_H_
#define _LCD_AN_TARGET_H_

#include <kernel.h>

#ifdef CONFIG_LCD_AN
#include <lcd_an_avr.h>

/* Hook-up LCD OS stack. */
#define LCD_AN_TGT_INIT()           lcd_an_avr_init()

#endif /* CONFIG_LCD_AN */

#endif /* _LCD_AN_TARGET_H_ */
