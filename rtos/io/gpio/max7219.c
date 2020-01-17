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

#ifdef CONFIG_MAX7219
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
    uint8_t i, buf[2];

    /* Initialize SPI device. */
    spi_init(&max->spi);

    /* Initialize MAX7219 message buffer. */
    spi_msg.buffer = buf;
    spi_msg.length = 2;
    spi_msg.flags = SPI_MSG_WRITE;

    /* Initialize MAX7219. */

    /* Turn off display. */
    buf[0] = MAX7219_ADDR_SHUTDOWN;
    buf[1] = 0x00;
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
        buf[1] = 0x07;
        status = spi_message(&max->spi, &spi_msg, 1);
    }


    if (status == SUCCESS)
    {
        /* Disable test mode. */
        buf[0] = MAX7219_ADDR_DISPLAY_TEST;
        buf[1] = 0xF0;
        status = spi_message(&max->spi, &spi_msg, 1);
    }


    if (status == SUCCESS)
    {
        /* Initialize with all zeros. */
        buf[1] = 0x00;
        for (i = 0; (status == SUCCESS) && (i < 8); i++)
        {
            buf[0] = (uint8_t)(i + 1);
            status = spi_message(&max->spi, &spi_msg, 1);
        }
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
        status = max7219_set_decode(&led_max->max, 0x00);
    }

    if (status == SUCCESS)
    {
        /* Turn on the display. */
        status = max7219_set_power(&led_max->max, TRUE);
    }

    /* Return status to the caller. */
    return (status);

} /* led_max7219_register */

#endif /* CONFIG_MAX7219 */
