/*
 * lcd_an_pcf8574.c
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
#include <kernel.h>

#ifdef CONFIG_LCD_AN
#ifdef CONFIG_LCD_PCF8574
#include <lcd_an_pcf8574.h>

/* Internal function prototypes. */
static void lcd_an_pcf8574_set_en(LCD_AN *);
static void lcd_an_pcf8574_clr_en(LCD_AN *);
static void lcd_an_pcf8574_set_rs(LCD_AN *);
static void lcd_an_pcf8574_clr_rs(LCD_AN *);
static void lcd_an_pcf8574_set_rw(LCD_AN *);
static void lcd_an_pcf8574_clr_rw(LCD_AN *);
static void lcd_an_pcf8574_put_data(LCD_AN *, uint8_t);
static uint8_t lcd_an_pcf8574_read_data(LCD_AN *);

/*
 * lcd_an_pcf8574_init
 * @lcd_an: PCF8574 LCD device.
 * This function is responsible for initializing alphanumeric LCD over
 * PCF8574 device.
 */
void lcd_an_pcf8574_init(LCD_AN_PCF8574 *lcd_an)
{
    /* Initialize GPIO controller. */
    if (pcf8574_init(&lcd_an->gpio) == SUCCESS)
    {
        /* Set driver hooks. */
        lcd_an->lcd.set_en     = &lcd_an_pcf8574_set_en;
        lcd_an->lcd.clr_en     = &lcd_an_pcf8574_clr_en;
        lcd_an->lcd.set_rs     = &lcd_an_pcf8574_set_rs;
        lcd_an->lcd.clr_rs     = &lcd_an_pcf8574_clr_rs;
        lcd_an->lcd.set_rw     = &lcd_an_pcf8574_set_rw;
        lcd_an->lcd.clr_rw     = &lcd_an_pcf8574_clr_rw;
        lcd_an->lcd.put_data   = &lcd_an_pcf8574_put_data;
        lcd_an->lcd.read_data  = &lcd_an_pcf8574_read_data;

        /* Configure all pins as output and high. */
        lcd_an->gpio.out_mask = lcd_an->gpio.out_data = 0xFF;

        /* Register PCF8574 LCD device. */
        lcd_an_register(&lcd_an->lcd);
    }

} /* lcd_pcf8574_init */

/*
 * lcd_an_pcf8574_set_en
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output high on enable pin.
 */
static void lcd_an_pcf8574_set_en(LCD_AN *lcd)
{
    LCD_AN_PCF8574 *lcd_pcf = (LCD_AN_PCF8574 *)lcd;

    /* Set the EN pin. */
    lcd_pcf->gpio.out_data |= (uint8_t)(1 << lcd_pcf->en_pin);

    /* Update the GPIO controller. */
    pcf8574_write(&lcd_pcf->gpio);

} /* lcd_an_pcf8574_set_en */

/*
 * lcd_an_pcf8574_clr_en
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output low on enable pin.
 */
static void lcd_an_pcf8574_clr_en(LCD_AN *lcd)
{
    LCD_AN_PCF8574 *lcd_pcf = (LCD_AN_PCF8574 *)lcd;

    /* Clear the EN pin. */
    lcd_pcf->gpio.out_data &= (uint8_t)(~(1 << lcd_pcf->en_pin));

    /* Update the GPIO controller. */
    pcf8574_write(&lcd_pcf->gpio);

} /* lcd_an_pcf8574_clr_en */

/*
 * lcd_an_pcf8574_set_rs
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output high on RS pin.
 */
static void lcd_an_pcf8574_set_rs(LCD_AN *lcd)
{
    LCD_AN_PCF8574 *lcd_pcf = (LCD_AN_PCF8574 *)lcd;

    /* Set the RS pin. */
    lcd_pcf->gpio.out_data |= (uint8_t)(1 << lcd_pcf->rs_pin);

    /* Update the GPIO controller. */
    pcf8574_write(&lcd_pcf->gpio);

} /* lcd_an_pcf8574_set_rs */

/*
 * lcd_an_pcf8574_clr_rs
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output low on RS pin.
 */
static void lcd_an_pcf8574_clr_rs(LCD_AN *lcd)
{
    LCD_AN_PCF8574 *lcd_pcf = (LCD_AN_PCF8574 *)lcd;

    /* Clear the RS pin. */
    lcd_pcf->gpio.out_data &= (uint8_t)(~(1 << lcd_pcf->rs_pin));

    /* Update the GPIO controller. */
    pcf8574_write(&lcd_pcf->gpio);

} /* lcd_an_pcf8574_clr_rs */

/*
 * lcd_an_pcf8574_set_rw
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output high on R/W pin.
 */
static void lcd_an_pcf8574_set_rw(LCD_AN *lcd)
{
    LCD_AN_PCF8574 *lcd_pcf = (LCD_AN_PCF8574 *)lcd;

    /* Set the RW pin. */
    lcd_pcf->gpio.out_data |= (uint8_t)(1 << lcd_pcf->rw_pin);

    /* Configure D7, D6, D5, D4 as input. */
    lcd_pcf->gpio.out_mask &= (uint8_t)(~((1 << lcd_pcf->d7_pin) | (1 << lcd_pcf->d6_pin) | (1 << lcd_pcf->d5_pin) | (1 << lcd_pcf->d4_pin)));

    /* Update the GPIO controller. */
    pcf8574_write(&lcd_pcf->gpio);

} /* lcd_an_pcf8574_set_rw */

/*
 * lcd_an_pcf8574_clr_rw
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output low on R/W pin.
 */
static void lcd_an_pcf8574_clr_rw(LCD_AN *lcd)
{
    LCD_AN_PCF8574 *lcd_pcf = (LCD_AN_PCF8574 *)lcd;

    /* Clear the RW pin. */
    lcd_pcf->gpio.out_data &= (uint8_t)(~(1 << lcd_pcf->rw_pin));

    /* Configure D7, D6, D5, D4 as output. */
    lcd_pcf->gpio.out_mask |= (uint8_t)((1 << lcd_pcf->d7_pin) | (1 << lcd_pcf->d6_pin) | (1 << lcd_pcf->d5_pin) | (1 << lcd_pcf->d4_pin));

    /* Update the GPIO controller. */
    pcf8574_write(&lcd_pcf->gpio);

} /* lcd_an_pcf8574_clr_rw */

/*
 * lcd_an_pcf8574_put_data
 * @lcd: LCD driver for which this function was called.
 * @nibble: Nibble to be put on the DAT pins.
 * This function is responsible for output a nibble on the DAT pins.
 */
static void lcd_an_pcf8574_put_data(LCD_AN *lcd, uint8_t nibble)
{
    LCD_AN_PCF8574 *lcd_pcf = (LCD_AN_PCF8574 *)lcd;

    if (nibble & (1 << 0))
    {
        /* Set D4 PIN. */
        lcd_pcf->gpio.out_data |= (uint8_t)(1 << lcd_pcf->d4_pin);
    }
    else
    {
        /* Clear D4 PIN. */
        lcd_pcf->gpio.out_data &= (uint8_t)(~(1 << lcd_pcf->d4_pin));
    }

    if (nibble & (1 << 1))
    {
        /* Set D5 PIN. */
        lcd_pcf->gpio.out_data |= (uint8_t)(1 << lcd_pcf->d5_pin);
    }
    else
    {
        /* Clear D5 PIN. */
        lcd_pcf->gpio.out_data &= (uint8_t)(~(1 << lcd_pcf->d5_pin));
    }

    if (nibble & (1 << 2))
    {
        /* Set D6 PIN. */
        lcd_pcf->gpio.out_data |= (uint8_t)(1 << lcd_pcf->d6_pin);
    }
    else
    {
        /* Clear D6 PIN. */
        lcd_pcf->gpio.out_data &= (uint8_t)(~(1 << lcd_pcf->d6_pin));
    }

    if (nibble & (1 << 3))
    {
        /* Set D7 PIN. */
        lcd_pcf->gpio.out_data |= (uint8_t)(1 << lcd_pcf->d7_pin);
    }
    else
    {
        /* Clear D7 PIN. */
        lcd_pcf->gpio.out_data &= (uint8_t)(~(1 << lcd_pcf->d7_pin));
    }

    /* Update the GPIO controller. */
    pcf8574_write(&lcd_pcf->gpio);

} /* lcd_an_pcf8574_put_data */

/*
 * lcd_an_pcf8574_read_data
 * @lcd: LCD driver for which this function was called.
 * @return: Nibble read from the DAT pins.
 * This function is responsible for input a nibble from the DAT pins.
 */
static uint8_t lcd_an_pcf8574_read_data(LCD_AN *lcd)
{
    uint8_t nibble = 0;
    LCD_AN_PCF8574 *lcd_pcf = (LCD_AN_PCF8574 *)lcd;
    uint8_t port = pcf8574_read(&lcd_pcf->gpio);

    /* Check if D4 is high. */
    if (port & (1 << lcd_pcf->d4_pin))
    {
        /* Set the 0th bit */
        nibble |= (1 << 0);
    }

    /* Check if D5 is high. */
    if (port & (1 << lcd_pcf->d5_pin))
    {
        /* Set the 1st bit */
        nibble |= (1 << 1);
    }

    /* Check if D6 is high. */
    if (port & (1 << lcd_pcf->d6_pin))
    {
        /* Set the 2nd bit */
        nibble |= (1 << 2);
    }

    /* Check if D7 is high. */
    if (port & (1 << lcd_pcf->d7_pin))
    {
        /* Set the 3rd bit */
        nibble |= (1 << 3);
    }

    /* Return the read nibble. */
    return (nibble);

} /* lcd_an_pcf8574_read_data */

#endif /* CONFIG_LCD_PCF8574 */
#endif /* CONFIG_LCD_AN */
