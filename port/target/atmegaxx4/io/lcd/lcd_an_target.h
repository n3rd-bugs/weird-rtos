/*
 * lcd_an_target.h
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _LCD_AN_TARGET_H_
#define _LCD_AN_TARGET_H_
#include <kernel.h>

#ifdef CONFIG_LCD_AN
#include <lcd_an_avr.h>

/* Hook-up LCD OS stack. */
#define LCD_AN_TGT_INIT()           lcd_an_avr_init()
#define LCD_AN_TGT_RESET()          lcd_an_avr_reset()

#endif /* CONFIG_LCD_AN */
#endif /* _LCD_AN_TARGET_H_ */
