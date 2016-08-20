/*
 * lcd_target.h
 *
 * Copyright (c) 2016 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */

#ifndef _LCD_TARGET_H_
#define _LCD_TARGET_H_

#include <os.h>

#ifdef CONFIG_LCD
#include <lcd_avr.h>

/* Hook-up LCD OS stack. */
#define LCD_TGT_INIT()          lcd_avr_init()
#define LCD_TGT_SET_EN(lcd)     lcd_avr_set_en(lcd)
#define LCD_TGT_CLR_EN(lcd)     lcd_avr_clr_en(lcd)
#define LCD_TGT_SET_RS(lcd)     lcd_avr_set_rs(lcd)
#define LCD_TGT_CLR_RS(lcd)     lcd_avr_clr_rs(lcd)
#define LCD_TGT_SET_RW(lcd)     lcd_avr_set_rw(lcd)
#define LCD_TGT_CLR_RW(lcd)     lcd_avr_clr_rw(lcd)
#define LCD_TGT_PUT_DAT(lcd, a) lcd_avr_put_data(lcd, a)
#define LCD_TGT_READ_DAT(lcd)   lcd_avr_read_data(lcd)

#endif /* CONFIG_LCD */

#endif /* _LCD_TARGET_H_ */
