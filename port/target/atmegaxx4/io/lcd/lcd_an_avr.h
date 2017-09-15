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
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */

#ifndef _LCD_AN_AVR_H_
#define _LCD_AN_AVR_H_

#include <kernel.h>

#ifdef CONFIG_LCD_AN
#include <lcd_an_target.h>

#ifndef CMAKE_BUILD
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

/* LCD I2C configuration. */
#define LCD_AN_AVR_I2C_ADDRESS      (0x3F)
#define LCD_AN_AVR_PIN_RW           (1)
#define LCD_AN_AVR_PIN_RS           (0)
#define LCD_AN_AVR_PIN_EN           (2)
#define LCD_AN_AVR_PIN_D4           (4)
#define LCD_AN_AVR_PIN_D5           (5)
#define LCD_AN_AVR_PIN_D6           (6)
#define LCD_AN_AVR_PIN_D7           (7)
#define LCD_AN_AVR_PIN_BL           (3)

#define LCD_AN_AVR_PIN_SCL          (0x3)
#define LCD_AN_AVR_PIN_SDA          (0x3)
#define LCD_AN_AVR_DDR_SCL          (0x4)
#define LCD_AN_AVR_DDR_SDA          (0x4)
#define LCD_AN_AVR_PORT_SCL         (0x5)
#define LCD_AN_AVR_PORT_SDA         (0x5)
#define LCD_AN_AVR_PIN_NUM_SCL      (0)
#define LCD_AN_AVR_PIN_NUM_SDA      (1)
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void lcd_an_avr_init(void);

#endif /* CONFIG_LCD_AN */

#endif /* _LCD_AN_AVR_H_ */
