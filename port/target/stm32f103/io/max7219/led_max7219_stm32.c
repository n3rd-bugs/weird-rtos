/*
 * led_max7219_stm32.c
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

#ifdef CONFIG_MAX7219
#ifdef CONFIG_SPI
#include <spi_stm32f103.h>
#else
#error "SPI is required for MAX7219."
#endif /* CONFIG_SPI */
#include <led_max7219_stm32.h>

/* Local variable definition. */
static STM32F103_SPI stm32_led_max7219_spi =
{
    /* Use the SPI1. */
    .device_num = 1,
};
static LED_MAX7219_STM32 stm32_led_max7219 =
{
    .led_max =
    {
        .max =
        {
            /* Hook-up SPI. */
            .spi =
            {
                 /* Assign function callback. */
                .init = &spi_stm32f103_init,
                .slave_select = &spi_stm32f103_slave_select,
                .slave_unselect = &spi_stm32f103_slave_unselect,
                .msg = &spi_stm32f103_message,
                .data = &stm32_led_max7219_spi,

                /* SPI configuration. */
                .baudrate = 10000000,
                .cfg_flags = (SPI_CFG_MASTER | SPI_CFG_CLK_FIRST_DATA),
            },
        },

        /* Graphics configurations. */
        .gfx =
        {
            /* Console configurations. */
            .console =
            {
                .fs =
                {
                    /* Name of this interface. */
                    .name = "8x8led1",
                },
            },

            /* Display dimensions. */
            .width      = 8,
            .height     = 8,
        },
    },
};

/*
 * led_max7219_stm32_init
 * This function is responsible for initializing LED over MAX7219 subsystem for STM32.
 */
void led_max7219_stm32_init(void)
{
    /* Register LED over MAX7219 device. */
    led_max7219_register(&stm32_led_max7219.led_max);

} /* led_max7219_stm32_init */

#endif /* CONFIG_MAX7219 */
