/*
 * lcd_an_avr.c
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
#include <kernel.h>

#ifdef CONFIG_LCD_AN
#include <lcd_an.h>
#include <lcd_an_avr.h>

#ifdef CONFIG_LCD_PCF8574
#include <i2c_bb_avr.h>
#include <lcd_an_pcf8574.h>

static I2C_BB_AVR avr_lcd_i2c =
{
    /* Initialize pin configuration. */
    .pin_scl        = LCD_AN_AVR_PIN_SCL,
    .pin_sda        = LCD_AN_AVR_PIN_SDA,
    .ddr_scl        = LCD_AN_AVR_DDR_SCL,
    .ddr_sda        = LCD_AN_AVR_DDR_SDA,
    .port_scl       = LCD_AN_AVR_PORT_SCL,
    .port_sda       = LCD_AN_AVR_PORT_SDA,
    .pin_num_scl    = LCD_AN_AVR_PIN_NUM_SCL,
    .pin_num_sda    = LCD_AN_AVR_PIN_NUM_SDA,
};
static LCD_AN_PCF8574 avr_lcd_an =
{
    /* LCD configuration. */
    .lcd =
    {
        .console =
        {
            .fs =
            {
                /* Name of this interface. */
                .name = "lcd1",
            },
        },

        .priv_data      = &avr_lcd_an,
        .row            = LCD_AN_AVR_ROWS,
        .column         = LCD_AN_AVR_COLS,

        .flags      = LCD_FLAG_DEBUG;
    },

    /* GPIO I2C configuration. */
    .gpio =
    {
        .i2c =
        {
            .data       = &avr_lcd_i2c,
            .init       = &i2c_bb_avr_init,
            .msg        = &i2c_bb_avr_message,
            .address    = LCD_AN_AVR_I2C_ADDRESS,
        },
    },

    /* PIN configuration. */
    .rw_pin             = LCD_AN_AVR_PIN_RW,
    .rs_pin             = LCD_AN_AVR_PIN_RS,
    .en_pin             = LCD_AN_AVR_PIN_EN,
    .d4_pin             = LCD_AN_AVR_PIN_D4,
    .d5_pin             = LCD_AN_AVR_PIN_D5,
    .d6_pin             = LCD_AN_AVR_PIN_D6,
    .d7_pin             = LCD_AN_AVR_PIN_D7,
    .bl_pin             = LCD_AN_AVR_PIN_BL,
};
#else
/* Internal function prototypes. */
static void lcd_an_avr_set_en(LCD_AN *);
static void lcd_an_avr_clr_en(LCD_AN *);
static void lcd_an_avr_set_rs(LCD_AN *);
static void lcd_an_avr_clr_rs(LCD_AN *);
static void lcd_an_avr_set_rw(LCD_AN *);
static void lcd_an_avr_clr_rw(LCD_AN *);
static void lcd_an_avr_put_data(LCD_AN *, uint8_t);
static uint8_t lcd_an_avr_read_data(LCD_AN *);

static LCD_AN avr_lcd_an =
{
    .console =
    {
        .fs =
        {
            /* Name of this interface. */
            .name = "lcd1",
        },
    },

    .priv_data  = NULL,

    /* Driver hooks. */
    .set_en     = &lcd_an_avr_set_en,
    .clr_en     = &lcd_an_avr_clr_en,
    .set_rs     = &lcd_an_avr_set_rs,
    .clr_rs     = &lcd_an_avr_clr_rs,
    .set_rw     = &lcd_an_avr_set_rw,
    .clr_rw     = &lcd_an_avr_clr_rw,
    .put_data   = &lcd_an_avr_put_data,
    .read_data  = &lcd_an_avr_read_data,

    .row        = LCD_AN_AVR_ROWS,
    .column     = LCD_AN_AVR_COLS,

    .flags      = LCD_FLAG_DEBUG;
};
#endif

/*
 * lcd_an_avr_init
 * This function is responsible for initializing LCD subsystem for AVR.
 */
void lcd_an_avr_init(void)
{
#ifdef CONFIG_LCD_PCF8574
    /* Register PCF8574 LCD device. */
    lcd_an_pcf8574_init(&avr_lcd_an);
#else
    /* Configure GPIO for LCD. */
    /* Configure D7 as output. */
    LCD_AN_AVR_D7_DDR |= (1 << LCD_AN_AVR_D7);

    /* Configure D6 as output. */
    LCD_AN_AVR_D6_DDR |= (1 << LCD_AN_AVR_D6);

    /* Configure D5 as output. */
    LCD_AN_AVR_D5_DDR |= (1 << LCD_AN_AVR_D5);

    /* Configure D4 as output. */
    LCD_AN_AVR_D4_DDR |= (1 << LCD_AN_AVR_D4);

    /* Configure EN as output. */
    LCD_AN_AVR_EN_DDR |= (1 << LCD_AN_AVR_EN);

    /* Configure RW as output. */
    LCD_AN_AVR_RW_DDR |= (1 << LCD_AN_AVR_RW);

    /* Configure RS as output. */
    LCD_AN_AVR_RS_DDR |= (1 << LCD_AN_AVR_RS);

    /* Register LCD device. */
    lcd_an_register(&avr_lcd_an);
#endif

} /* lcd_an_avr_init */


/*
 * lcd_an_avr_reset
 * This function will reset the LCD interface.
 */
int32_t lcd_an_avr_reset(void)
{
    int32_t status = SUCCESS;

#ifdef CONFIG_LCD_PCF8574
    /* Reset the PCF8574 LCD device. */
    status = lcd_an_pcf8574_reset(&avr_lcd_an);
#endif

    /* Return status to the caller. */
    return (status);

} /* lcd_an_avr_reset */

#ifndef CONFIG_LCD_PCF8574
/*
 * lcd_an_avr_set_en
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output high on enable pin.
 */
static void lcd_an_avr_set_en(LCD_AN *lcd)
{
    UNUSED_PARAM(lcd);

    /* Set enable PIN. */
    LCD_AN_AVR_EN_PORT |= (1 << LCD_AN_AVR_EN);

} /* lcd_an_avr_set_en */

/*
 * lcd_an_avr_clr_en
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output low on enable pin.
 */
static void lcd_an_avr_clr_en(LCD_AN *lcd)
{
    UNUSED_PARAM(lcd);

    /* Clear enable PIN. */
    LCD_AN_AVR_EN_PORT &= (uint8_t)(~(1 << LCD_AN_AVR_EN));

} /* lcd_an_avr_clr_en */

/*
 * lcd_an_avr_set_rs
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output high on RS pin.
 */
static void lcd_an_avr_set_rs(LCD_AN *lcd)
{
    UNUSED_PARAM(lcd);

    /* Set R/S PIN. */
    LCD_AN_AVR_RS_PORT |= (1 << LCD_AN_AVR_RS);

} /* lcd_an_avr_set_rs */

/*
 * lcd_an_avr_clr_rs
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output low on RS pin.
 */
static void lcd_an_avr_clr_rs(LCD_AN *lcd)
{
    UNUSED_PARAM(lcd);

    /* Clear R/S PIN. */
    LCD_AN_AVR_RS_PORT &= (uint8_t)(~(1 << LCD_AN_AVR_RS));

} /* lcd_an_avr_clr_rs */

/*
 * lcd_an_avr_set_rw
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output high on R/W pin.
 */
static void lcd_an_avr_set_rw(LCD_AN *lcd)
{
    UNUSED_PARAM(lcd);

    /* Set R/W PIN. */
    LCD_AN_AVR_RW_PORT |= (1 << LCD_AN_AVR_RW);

    /* Configure D7 as input. */
    LCD_AN_AVR_D7_DDR &= (uint8_t)(~(1 << LCD_AN_AVR_D7));

    /* Configure D6 as input. */
    LCD_AN_AVR_D6_DDR &= (uint8_t)(~(1 << LCD_AN_AVR_D6));

    /* Configure D5 as input. */
    LCD_AN_AVR_D5_DDR &= (uint8_t)(~(1 << LCD_AN_AVR_D5));

    /* Configure D4 as input. */
    LCD_AN_AVR_D4_DDR &= (uint8_t)(~(1 << LCD_AN_AVR_D4));

} /* lcd_an_avr_set_rw */

/*
 * lcd_an_avr_clr_rw
 * @lcd: LCD driver for which this function was called.
 * This function is responsible for output low on R/W pin.
 */
static void lcd_an_avr_clr_rw(LCD_AN *lcd)
{
    UNUSED_PARAM(lcd);

    /* Clear R/W PIN. */
    LCD_AN_AVR_RW_PORT &= (uint8_t)(~(1 << LCD_AN_AVR_RW));

    /* Configure D7 as output. */
    LCD_AN_AVR_D7_DDR |= (1 << LCD_AN_AVR_D7);

    /* Configure D6 as output. */
    LCD_AN_AVR_D6_DDR |= (1 << LCD_AN_AVR_D6);

    /* Configure D5 as output. */
    LCD_AN_AVR_D5_DDR |= (1 << LCD_AN_AVR_D5);

    /* Configure D4 as output. */
    LCD_AN_AVR_D4_DDR |= (1 << LCD_AN_AVR_D4);

} /* lcd_an_avr_clr_rw */

/*
 * lcd_an_avr_put_data
 * @lcd: LCD driver for which this function was called.
 * @nibble: Nibble to be put on the DAT pins.
 * This function is responsible for output a nibble on the DAT pins.
 */
static void lcd_an_avr_put_data(LCD_AN *lcd, uint8_t nibble)
{
    UNUSED_PARAM(lcd);

    if (nibble & (1 << 0))
    {
        /* Set D4 PIN. */
        LCD_AN_AVR_D4_PORT |= (1 << LCD_AN_AVR_D4);
    }
    else
    {
        /* Clear D4 PIN. */
        LCD_AN_AVR_D4_PORT &= (uint8_t)(~(1 << LCD_AN_AVR_D4));
    }

    if (nibble & (1 << 1))
    {
        /* Set D5 PIN. */
        LCD_AN_AVR_D5_PORT |= (1 << LCD_AN_AVR_D5);
    }
    else
    {
        /* Clear D5 PIN. */
        LCD_AN_AVR_D5_PORT &= (uint8_t)(~(1 << LCD_AN_AVR_D5));
    }

    if (nibble & (1 << 2))
    {
        /* Set D6 PIN. */
        LCD_AN_AVR_D6_PORT |= (1 << LCD_AN_AVR_D6);
    }
    else
    {
        /* Clear D6 PIN. */
        LCD_AN_AVR_D6_PORT &= (uint8_t)(~(1 << LCD_AN_AVR_D6));
    }

    if (nibble & (1 << 3))
    {
        /* Set D7 PIN. */
        LCD_AN_AVR_D7_PORT |= (1 << LCD_AN_AVR_D7);
    }
    else
    {
        /* Clear D7 PIN. */
        LCD_AN_AVR_D7_PORT &= (uint8_t)(~(1 << LCD_AN_AVR_D7));
    }

} /* lcd_an_avr_put_data */

/*
 * lcd_an_avr_read_data
 * @lcd: LCD driver for which this function was called.
 * @return: Nibble read from the DAT pins.
 * This function is responsible for input a nibble from the DAT pins.
 */
static uint8_t lcd_an_avr_read_data(LCD_AN *lcd)
{
    uint8_t nibble = 0;
    UNUSED_PARAM(lcd);

    /* Check if D4 is high. */
    if (LCD_AN_AVR_D4_PIN & (1 << LCD_AN_AVR_D4))
    {
        /* Set the 0th bit */
        nibble |= (1 << 0);
    }

    /* Check if D5 is high. */
    if (LCD_AN_AVR_D5_PIN & (1 << LCD_AN_AVR_D5))
    {
        /* Set the 1st bit */
        nibble |= (1 << 1);
    }

    /* Check if D6 is high. */
    if (LCD_AN_AVR_D6_PIN & (1 << LCD_AN_AVR_D6))
    {
        /* Set the 2nd bit */
        nibble |= (1 << 2);
    }

    /* Check if D7 is high. */
    if (LCD_AN_AVR_D7_PIN & (1 << LCD_AN_AVR_D7))
    {
        /* Set the 3rd bit */
        nibble |= (1 << 3);
    }

    /* Return the read nibble. */
    return (nibble);

} /* lcd_an_avr_read_data */
#endif /* CONFIG_LCD_PCF8574 */

#endif /* CONFIG_LCD_AN */
