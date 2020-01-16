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
    SPI_MSG spi_msg;
    uint8_t buf[2];

    /* Initialize SPI device. */
    spi_init(&max->spi);

    /* Initialize SPI message. */
    spi_msg.buffer = buf;
    buf[0] = 0x1;
    buf[1] = 0x0F;
    spi_msg.length = 2;
    spi_msg.flags = SPI_MSG_WRITE;

    /* Send a message on SPI bus. */
    return (spi_message(&max->spi, &spi_msg, 1));

} /* max7219_register */

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

    /* Return status to the caller. */
    return (status);

} /* led_max7219_register */

#endif /* CONFIG_MAX7219 */
