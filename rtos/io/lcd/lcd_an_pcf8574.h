/*
 * lcd_an_pcf8574.h
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
int32_t lcd_an_pcf8574_reset(LCD_AN_PCF8574 *);

#endif /* CONFIG_LCD_PCF8574 */
#endif /* CONFIG_LCD_AN */
#endif /* _LCD_AN_PCF8574_H_ */
