/*
 * oled_avr.c
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
#include "../../../../../rtos/io/oled/oled_ssd1306.h"
#include <oled_avr.h>
#include <i2c_bb_avr.h>

/* OLED I2C device. */
static I2C_BB_AVR avr_oled_i2c =
{
    /* Initialize pin configuration. */
    .pin_scl        = OLED_AVR_PIN_SCL,
    .pin_sda        = OLED_AVR_PIN_SDA,
    .ddr_scl        = OLED_AVR_DDR_SCL,
    .ddr_sda        = OLED_AVR_DDR_SDA,
    .port_scl       = OLED_AVR_PORT_SCL,
    .port_sda       = OLED_AVR_PORT_SDA,
    .pin_num_scl    = OLED_AVR_PIN_NUM_SCL,
    .pin_num_sda    = OLED_AVR_PIN_NUM_SDA,
};
static SSD1306 avr_ssd1306 =
{
    /* I2C configuration. */
    .i2c =
    {
        .data       = &avr_oled_i2c,
        .init       = &i2c_bb_avr_init,
        .msg        = &i2c_bb_avr_message,
        .address    = OLED_AVR_I2C_ADDRESS,
    },

    /* OLED configuration. */
    .width          = 128,
    .height         = 64,
    .flags          = 0,
};

/*
 * oled_avr_init
 * This function is responsible for initializing OLED subsystem for AVR.
 */
void oled_avr_init(void)
{
    /* Register this OLED device. */
    oled_ssd1306_register(&avr_ssd1306);

} /* oled_avr_init */
#endif /* CONFIG_OLED */
