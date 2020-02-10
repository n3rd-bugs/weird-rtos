/*
 * lcd_an_avr.h
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
#ifndef _LCD_AN_AVR_H_
#define _LCD_AN_AVR_H_
#include <kernel.h>

#ifdef IO_LCD_AN
#include <lcd_an_target.h>
#include <lcd_an_avr_config.h>

/* Function prototypes. */
void lcd_an_avr_init(void);
int32_t lcd_an_avr_reset(void);

#endif /* IO_LCD_AN */
#endif /* _LCD_AN_AVR_H_ */
