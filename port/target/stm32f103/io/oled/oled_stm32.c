/*
 * oled_stm32.c
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

#ifdef CONFIG_OLED
#include <oled_ssd1306.h>
#include <oled_stm32.h>
#include <i2c_stm32f103.h>

/* OLED I2C device. */
static I2C_STM32 stm32_lcd_i2c =
{
    /* Initialize I2C configuration. */
    .speed          = 400000,
    .device_num     = 1,
};
static SSD1306 stm32_ssd1306 =
{
    /* I2C configuration. */
    .i2c =
    {
        .data       = &stm32_lcd_i2c,
        .init       = &i2c_stm32f103_init,
        .msg        = &i2c_stm32f103_message,
        .address    = OLED_STM32_I2C_ADDRESS,
    },

    /* OLED configuration. */
    .width          = 128,
    .height         = 64,
    .flags          = 0,
};

/*
 * oled_stm32_init
 * This function is responsible for initializing OLED subsystem for STM32.
 */
void oled_stm32_init(void)
{
    /* Register this OLED device. */
    oled_ssd1306_register(&stm32_ssd1306);

} /* oled_stm32_init */
#endif /* CONFIG_OLED */
