/*
 * lcd_avr.h
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

#ifndef _LCD_AVR_H_
#define _LCD_AVR_H_

#include <os.h>

#ifdef CONFIG_LCD

/* LCD configuration for AVR. */
/* Number of rows and columns on this LCD. */
#define LCD_AVR_ROWS        (4)
#define LCD_AVR_COLS        (16)

/* Enable PIN configuration. */
#define LCD_AVR_EN          (1)
#define LCD_AVR_EN_PORT     (PORTC)
#define LCD_AVR_EN_DDR      (DDRC)
#define LCD_AVR_EN_PIN      (PINC)

/* Read/write PIN configuration. */
#define LCD_AVR_RW          (0)
#define LCD_AVR_RW_PORT     (PORTC)
#define LCD_AVR_RW_DDR      (DDRC)
#define LCD_AVR_RW_PIN      (PINC)

/* Register select PIN configuration. */
#define LCD_AVR_RS          (7)
#define LCD_AVR_RS_PORT     (PORTD)
#define LCD_AVR_RS_DDR      (DDRD)
#define LCD_AVR_RS_PIN      (PIND)

/* DAT7 PIN configuration. */
#define LCD_AVR_D7          (5)
#define LCD_AVR_D7_PORT     (PORTD)
#define LCD_AVR_D7_DDR      (DDRD)
#define LCD_AVR_D7_PIN      (PIND)

/* DAT6 PIN configuration. */
#define LCD_AVR_D6          (6)
#define LCD_AVR_D6_PORT     (PORTD)
#define LCD_AVR_D6_DDR      (DDRD)
#define LCD_AVR_D6_PIN      (PIND)

/* DAT5 PIN configuration. */
#define LCD_AVR_D5          (6)
#define LCD_AVR_D5_PORT     (PORTC)
#define LCD_AVR_D5_DDR      (DDRC)
#define LCD_AVR_D5_PIN      (PINC)

/* DAT4 PIN configuration. */
#define LCD_AVR_D4          (2)
#define LCD_AVR_D4_PORT     (PORTC)
#define LCD_AVR_D4_DDR      (DDRC)
#define LCD_AVR_D4_PIN      (PINC)

/* Function prototypes. */
void lcd_avr_init();
void lcd_avr_set_en(LCD *);
void lcd_avr_clr_en(LCD *);
void lcd_avr_set_rs(LCD *);
void lcd_avr_clr_rs(LCD *);
void lcd_avr_set_rw(LCD *);
void lcd_avr_clr_rw(LCD *);
void lcd_avr_put_data(LCD *, uint8_t);
uint8_t lcd_avr_read_data(LCD *);

#endif /* CONFIG_LCD */

#endif /* _LCD_AVR_H_ */
