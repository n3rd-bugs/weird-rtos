/*
 * lcd_an_avr.h
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

#ifndef _LCD_AN_AVR_H_
#define _LCD_AN_AVR_H_

#include <os.h>

#ifdef CONFIG_LCD_AN

/* LCD configuration for AVR. */
/* Number of rows and columns on this LCD. */
#define LCD_AN_AVR_ROWS     (4)
#define LCD_AN_AVR_COLS     (20)

/* Register select PIN configuration. */
#define LCD_AN_AVR_RS       (5)
#define LCD_AN_AVR_RS_PORT  (PORTD)
#define LCD_AN_AVR_RS_DDR   (DDRD)
#define LCD_AN_AVR_RS_PIN   (PIND)

/* Read/write PIN configuration. */
#define LCD_AN_AVR_RW       (6)
#define LCD_AN_AVR_RW_PORT  (PORTD)
#define LCD_AN_AVR_RW_DDR   (DDRD)
#define LCD_AN_AVR_RW_PIN   (PIND)

/* Enable PIN configuration. */
#define LCD_AN_AVR_EN       (6)
#define LCD_AN_AVR_EN_PORT  (PORTC)
#define LCD_AN_AVR_EN_DDR   (DDRC)
#define LCD_AN_AVR_EN_PIN   (PINC)

/* DAT4 PIN configuration. */
#define LCD_AN_AVR_D4       (2)
#define LCD_AN_AVR_D4_PORT  (PORTC)
#define LCD_AN_AVR_D4_DDR   (DDRC)
#define LCD_AN_AVR_D4_PIN   (PINC)

/* DAT5 PIN configuration. */
#define LCD_AN_AVR_D5       (1)
#define LCD_AN_AVR_D5_PORT  (PORTC)
#define LCD_AN_AVR_D5_DDR   (DDRC)
#define LCD_AN_AVR_D5_PIN   (PINC)

/* DAT6 PIN configuration. */
#define LCD_AN_AVR_D6       (0)
#define LCD_AN_AVR_D6_PORT  (PORTC)
#define LCD_AN_AVR_D6_DDR   (DDRC)
#define LCD_AN_AVR_D6_PIN   (PINC)

/* DAT7 PIN configuration. */
#define LCD_AN_AVR_D7       (7)
#define LCD_AN_AVR_D7_PORT  (PORTD)
#define LCD_AN_AVR_D7_DDR   (DDRD)
#define LCD_AN_AVR_D7_PIN   (PIND)

/* Function prototypes. */
void lcd_an_avr_init();
void lcd_an_avr_set_en(LCD_AN *);
void lcd_an_avr_clr_en(LCD_AN *);
void lcd_an_avr_set_rs(LCD_AN *);
void lcd_an_avr_clr_rs(LCD_AN *);
void lcd_an_avr_set_rw(LCD_AN *);
void lcd_an_avr_clr_rw(LCD_AN *);
void lcd_an_avr_put_data(LCD_AN *, uint8_t);
uint8_t lcd_an_avr_read_data(LCD_AN *);

#endif /* CONFIG_LCD_AN */

#endif /* _LCD_AN_AVR_H_ */
