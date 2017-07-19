/*
 * lcd_an_pcf8574.h
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

#ifndef _LCD_AN_PCF8574_H_
#define _LCD_AN_PCF8574_H_

#include <kernel.h>

#ifdef CONFIG_LCD_AN
#ifdef CONFIG_LCD_PCF8574
#ifndef CONFIG_PCF8574
#error "PCF8574 driver is required for this driver."
#endif
#include <lcd_an.h>
#include <pcf8574.h>

/* This defines alphanumeric LCD over PCF8574 device. */
typedef struct lcd_an_pcf8574
{
    /* LCD device. */
    LCD_AN      lcd;

    /* PCF8574 device. */
    PCF8574     gpio;

    /* PIN configurations. */
    uint8_t     rs_pin;
    uint8_t     rw_pin;
    uint8_t     en_pin;
    uint8_t     d4_pin;
    uint8_t     d5_pin;
    uint8_t     d6_pin;
    uint8_t     d7_pin;
    uint8_t     bl_pin;

} LCD_AN_PCF8574;

/* Function prototypes. */
void lcd_an_pcf8574_init(LCD_AN_PCF8574 *);

#endif /* CONFIG_LCD_PCF8574 */
#endif /* CONFIG_LCD_AN */
#endif /* _LCD_AN_PCF8574_H_ */
