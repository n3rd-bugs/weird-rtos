/*
 * lcd_an_stm32.c
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
#include <kernel.h>

#ifdef CONFIG_LCD_AN
#include <lcd_an.h>
#include <lcd_an_stm32.h>

#ifdef CONFIG_LCD_PCF8574
#include <i2c_stm32f103.h>
#include <lcd_an_pcf8574.h>

static I2C_STM32 stm32_lcd_i2c =
{
    /* Initialize I2C configuration. */
    .speed          = 400000,
    .device_num     = 1,
};
static LCD_AN_PCF8574 stm32_lcd_an =
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

        .priv_data      = &stm32_lcd_an,
        .row            = LCD_AN_STM32_ROWS,
        .column         = LCD_AN_STM32_COLS,
    },

    /* GPIO I2C configuration. */
    .gpio =
    {
        .i2c =
        {
            .data       = &stm32_lcd_i2c,
            .init       = &i2c_stm32f103_init,
            .msg        = &i2c_stm32f103_message,
            .address    = LCD_AN_STM32_I2C_ADDRESS,
        },
    },

    /* PIN configuration. */
    .rw_pin             = LCD_AN_STM32_PIN_RW,
    .rs_pin             = LCD_AN_STM32_PIN_RS,
    .en_pin             = LCD_AN_STM32_PIN_EN,
    .d4_pin             = LCD_AN_STM32_PIN_D4,
    .d5_pin             = LCD_AN_STM32_PIN_D5,
    .d6_pin             = LCD_AN_STM32_PIN_D6,
    .d7_pin             = LCD_AN_STM32_PIN_D7,
    .bl_pin             = LCD_AN_STM32_PIN_BL,
};
#endif

/*
 * lcd_an_stm32_init
 * This function is responsible for initializing LCD subsystem for STM32.
 */
void lcd_an_stm32_init(void)
{
#ifdef CONFIG_LCD_PCF8574
    /* Register PCF8574 LCD device. */
    lcd_an_pcf8574_init(&stm32_lcd_an);
#endif

} /* lcd_an_stm32_init */
#endif /* CONFIG_LCD_AN */
