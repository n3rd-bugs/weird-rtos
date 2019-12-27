/*
 * lcd_an_stm32.c
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
#include <kernel.h>

#ifdef CONFIG_LCD_AN
#include <lcd_an.h>
#include <lcd_an_stm32.h>

#ifdef CONFIG_LCD_PCF8574
#include <i2c_stm32f030.h>
#include <lcd_an_pcf8574.h>

static I2C_STM32 stm32_lcd_i2c =
{
    /* Initialize I2C configuration. */
    .timing         = 0x20303E5D,
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
            .init       = &i2c_stm32f030_init,
            .msg        = &i2c_stm32f030_message,
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
