/*
 * lcd_an_stm32.h
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
#ifndef _LCD_AN_STM32_H_
#define _LCD_AN_STM32_H_

#include <kernel.h>

#ifdef CONFIG_LCD_AN
#include <lcd_an_target.h>
#include <lcd_an_stm32_config.h>

/* Function prototypes. */
void lcd_an_stm32_init(void);

#endif /* CONFIG_LCD_AN */
#endif /* _LCD_AN_STM32_H_ */
