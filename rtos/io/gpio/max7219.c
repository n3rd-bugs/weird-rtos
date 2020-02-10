/*
 * max7219.c
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
#include <max7219.h>
#include <max7219_target.h>

#ifdef GPIO_MAX7219
/* Local function definition. */
static int32_t led_max_ssd1306_display(GFX *, const uint8_t *, uint32_t, uint32_t, uint32_t, uint32_t);
static int32_t led_max_ssd1306_power(GFX *, uint8_t);
static int32_t led_max_ssd1306_clear_display(GFX *);
static int32_t led_max_ssd1306_invert(GFX *, uint8_t);

/*
 * max7219_init
 * This will initialize MAX7219 device subsystem.
 */
void max7219_init(void)
{
    /* Call target initialization routines. */
#ifdef MAX7219_TGT_INIT
    MAX7219_TGT_INIT();
#endif /* MAX7219_TGT_INIT */
#ifdef LED_MAX7219_TGT_INIT
    LED_MAX7219_TGT_INIT();
#endif /* LED_MAX7219_TGT_INIT */
} /* max7219_init */

/*
 * max7219_register
 * @max: MAX7219 device needed to be registered.
 * @return: Success will be returned if MAX7219 device was successfully
 *  Initialized.
 * This function will initialize a MAX7219 device.
 */
int32_t max7219_register(MAX7219 *max)
{
    int32_t status;
    SPI_MSG spi_msg;
    uint8_t buf[2];

    /* Initialize SPI device. */
    spi_init(&max->spi);

    /* Initialize MAX7219 message buffer. */
    spi_msg.buffer = buf;
    spi_msg.length = 2;
    spi_msg.flags = SPI_MSG_WRITE;

    /* Initialize MAX7219. */

    /* Turn off display. */
    buf[0] = MAX7219_ADDR_SHUTDOWN;
    buf[1] = 0x0;
    status = spi_message(&max->spi, &spi_msg, 1);

    if (status == SUCCESS)
    {
        /* Configure intensity. */
        buf[0] = MAX7219_ADDR_INTENSITY;
        buf[1] = MAX7219_INTENSITY;
        status = spi_message(&max->spi, &spi_msg, 1);
    }


    if (status == SUCCESS)
    {
        /* Configure scan limit [0-7 digits]. */
        buf[0] = MAX7219_ADDR_SCAN_LIMIT;
        buf[1] = 0x7;
        status = spi_message(&max->spi, &spi_msg, 1);
    }


    if (status == SUCCESS)
    {
        /* Disable test mode. */
        buf[0] = MAX7219_ADDR_DISPLAY_TEST;
        buf[1] = 0xF0;
        status = spi_message(&max->spi, &spi_msg, 1);
    }

    /* Return status to the caller. */
    return (status);

} /* max7219_register */

/*
 * max7219_set_power
 * @max: MAX7219 device.
 * @power: Power state needed to be set.
 * @return: Success will be returned if display power was successfully set
 * This function will set the power state for the MAX7219 device.
 */
int32_t max7219_set_power(MAX7219 *max, uint8_t power)
{
    uint8_t buf[2] = {MAX7219_ADDR_SHUTDOWN, power};
    SPI_MSG spi_msg =
    {
        .buffer = buf,
        .length = 2,
        .flags = SPI_MSG_WRITE,
    };

    /* Return status to the caller. */
    return (spi_message(&max->spi, &spi_msg, 1));

} /* max7219_set_power */

/*
 * max7219_set_decode
 * @max: MAX7219 device.
 * @decode: Decode mode needed to be set.
 * @return: Success will be returned if decode mode was successfully set
 * This function will set the decode mode for the MAX7219 device.
 */
int32_t max7219_set_decode(MAX7219 *max, uint8_t decode)
{
    uint8_t buf[2] = {MAX7219_ADDR_SHUTDOWN, decode};
    SPI_MSG spi_msg =
    {
        .buffer = buf,
        .length = 2,
        .flags = SPI_MSG_WRITE,
    };

    /* Return status to the caller. */
    return (spi_message(&max->spi, &spi_msg, 1));

} /* max7219_set_decode */

/*
 * led_max7219_register
 * @device: LED over MAX7219 device needed to be registered.
 * @return: Success will be returned if LED over MAX7219 device was successfully
 *  Initialized.
 * This function will initialize a LED over MAX7219 device.
 */
int32_t led_max7219_register(LED_MAX7219 *led_max)
{
    int32_t status;

    /* Register MAX7219 device. */
    status = max7219_register(&led_max->max);

    if (status == SUCCESS)
    {
        /* Disable the decode mode. */
        status = max7219_set_decode(&led_max->max, 0x0);
    }

    if (status == SUCCESS)
    {
        /* Turn on the display. */
        status = max7219_set_power(&led_max->max, TRUE);
    }

    if (status == SUCCESS)
    {
        /* Hook up graphics for this OLED. */
        led_max->gfx.display = &led_max_ssd1306_display;
        led_max->gfx.power = &led_max_ssd1306_power;
        led_max->gfx.clear = &led_max_ssd1306_clear_display;
        led_max->gfx.invert = &led_max_ssd1306_invert;

        /* Set the font information. */
        led_max->gfx.font = NULL;

        /* Register this with graphics driver. */
        graphics_register(&led_max->gfx);
    }

    /* Return status to the caller. */
    return (status);

} /* led_max7219_register */

/*
 * led_max_ssd1306_display
 * @gfx: Graphics data.
 * @buffer: Buffer to display.
 * @col: Starting column.
 * @num_col: Number of columns.
 * @row: Starting row.
 * @num_row: Number of rows.
 * @return: Success will be returned if the given data was successfully displayed,
 *  MAX7219_OUT_OF_RANGE will be returned if column or row is out of range.
 * This function will display a buffer on LED.
 */
static int32_t led_max_ssd1306_display(GFX *gfx, const uint8_t *buffer, uint32_t col, uint32_t num_col, uint32_t row, uint32_t num_row)
{
    LED_MAX7219 *led_max = (LED_MAX7219 *)gfx;
    int32_t status = SUCCESS;
    uint8_t i, buf[2];
    SPI_MSG spi_msg;

    /* Validate input parameters. */
    if (((row + num_row) > gfx->height) || ((col + num_col) > gfx->width))
    {
        /* Return error. */
        status = MAX7219_OUT_OF_RANGE;
    }
    else
    {
        /* Initialize MAX7219 message buffer. */
        spi_msg.buffer = buf;
        spi_msg.length = 2;
        spi_msg.flags = SPI_MSG_WRITE;

        /* Flush the buffer. */
        for (i = (uint8_t)col; (status == SUCCESS) && (i < (uint8_t)(col + num_col)); i++)
        {
            /* Populate the SPI buffer. */
            buf[0] = (uint8_t)(i + 1);
            buf[1] = buffer[i];

            /* Send SPI message. */
            status = spi_message(&led_max->max.spi, &spi_msg, 1);
        }
    }

    /* Return status to the caller. */
    return (status);

} /* led_max_ssd1306_display */

/*
 * oled_ssd1306_power
 * @gfx: Graphics data.
 * @turn_on: If we are needed to turn on the display.
 * @return: Success will be returned if power state was successfully updated.
 * This function will clear the display.
 */
static int32_t led_max_ssd1306_power(GFX *gfx, uint8_t turn_on)
{
    LED_MAX7219 *led_max = (LED_MAX7219 *)gfx;

    /* Return status to the caller. */
    return (max7219_set_power(&led_max->max, turn_on));

} /* led_max_ssd1306_power */

/*
 * oled_ssd1306_clear_display
 * @gfx: Graphics data.
 * @return: Success will be returned if display was successfully cleared.
 * This function will clear the display.
 */
static int32_t led_max_ssd1306_clear_display(GFX *gfx)
{
    LED_MAX7219 *led_max = (LED_MAX7219 *)gfx;
    int32_t status = SUCCESS;
    uint8_t i, buf[2];
    SPI_MSG spi_msg;

    /* Initialize MAX7219 message buffer. */
    spi_msg.buffer = buf;
    spi_msg.length = 2;
    spi_msg.flags = SPI_MSG_WRITE;

    /* Initialize with all zeros. */
    buf[1] = 0x0;
    for (i = 0; (status == SUCCESS) && (i < 8); i++)
    {
        buf[0] = (uint8_t)(i + 1);
        status = spi_message(&led_max->max.spi, &spi_msg, 1);
    }

    /* Return status to the caller. */
    return (status);

} /* led_max_ssd1306_clear_display */

/*
 * oled_ssd1306_invert
 * @gfx: Graphics data.
 * @invert: Flag to specify if we need to invert the display.
 * @return: Always return MAX7219_NOT_SUPPORTED.
 * This function will invert the display.
 */
static int32_t led_max_ssd1306_invert(GFX *gfx, uint8_t invert)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(gfx);
    UNUSED_PARAM(invert);

    /* Invert not supported. */
    return (MAX7219_NOT_SUPPORTED);

} /* led_max_ssd1306_invert */

#endif /* GPIO_MAX7219 */
