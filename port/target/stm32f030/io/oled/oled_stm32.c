/*
 * oled_stm32.c
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

#ifdef CONFIG_OLED
#include <oled_ssd1306.h>
#include <oled_stm32.h>
#include <i2c_stm32f030.h>

/* OLED I2C device. */
static I2C_STM32 stm32_lcd_i2c =
{
    /* Initialize I2C configuration. */
    .timing         = 0x20303E5D,
    .device_num     = 1,
};
static SSD1306 stm32_ssd1306 =
{
    /* Graphics configurations. */
    .gfx =
    {
        /* Console configurations. */
        .console =
        {
            .fs =
            {
                /* Name of this interface. */
                .name = "oled1",
            },
        },

        /* Display dimensions. */
        .width      = 128,
        .height     = 64,

        /* Use this for debug interface. */
        .flags      = GFX_FLAG_DEBUG,
    },

    /* I2C configuration. */
    .i2c =
    {
        .data       = &stm32_lcd_i2c,
        .init       = &i2c_stm32f030_init,
        .msg        = &i2c_stm32f030_message,
        .address    = OLED_STM32_I2C_ADDRESS,
    },

    /* OLED configuration. */
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
