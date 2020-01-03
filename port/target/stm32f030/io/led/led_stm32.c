/*
 * led_stm32.c
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

#ifdef CONFIG_LED
#include <led_ssd1306.h>
#include <led_stm32.h>
#include <spi_stm32f030.h>

/* LED SPI device. */
static SPI_STM32 stm32_lcd_spi =
{
    /* Initialize SPI configuration. */
    .device_num     = 1,
};
static LED_MAX7219 stm32_ssd1306 =
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
                .name = "led1",
            },
        },

        /* Display dimensions. */
        .width      = 8,
        .height     = 8,
    },

    /* SPI configuration. */
    .spi =
    {
        .data       = &stm32_lcd_i2c,
        .init       = &i2c_stm32f030_init,
        .msg        = &i2c_stm32f030_message,
        .address    = LED_STM32_I2C_ADDRESS,
    },

    /* LED configuration. */
    .flags          = 0,
};

/*
 * led_stm32_init
 * This function is responsible for initializing LED subsystem for STM32.
 */
void led_stm32_init(void)
{
    /* Register this LED device. */
    led_ssd1306_register(&stm32_ssd1306);

} /* led_stm32_init */
#endif /* CONFIG_LED */
