/*
 * lcd_an_stm32.h
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
#ifndef _LCD_AN_STM32_H_
#define _LCD_AN_STM32_H_

#include <kernel.h>

#ifdef CONFIG_LCD_AN
#include <lcd_an_target.h>

#ifndef CMAKE_BUILD
/* LCD configuration for STM32. */
/* Number of rows and columns on this LCD. */
#define LCD_AN_STM32_ROWS     (4)
#define LCD_AN_STM32_COLS     (20)

/* LCD I2C configuration. */
#define LCD_AN_STM32_I2C_ADDRESS      (0x3F)
#define LCD_AN_STM32_PIN_RW           (1)
#define LCD_AN_STM32_PIN_RS           (0)
#define LCD_AN_STM32_PIN_EN           (2)
#define LCD_AN_STM32_PIN_D4           (4)
#define LCD_AN_STM32_PIN_D5           (5)
#define LCD_AN_STM32_PIN_D6           (6)
#define LCD_AN_STM32_PIN_D7           (7)
#define LCD_AN_STM32_PIN_BL           (3)
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void lcd_an_stm32_init(void);

#endif /* CONFIG_LCD_AN */
#endif /* _LCD_AN_STM32_H_ */
