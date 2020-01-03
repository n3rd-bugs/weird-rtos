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

#ifdef CONFIG_MAX7219
/*
 * max7219_init
 * @device: MAX7219 device needed to be initialized.
 * @return: Success will be returned if MAX7219 device was successfully
 *  Initialized.
 * This function will initialize an MAX7219 device.
 */
int32_t max7219_init(MAX7219 *device)
{
    SPI_MSG spi_msg;

    /* Initialize SPI device. */
    spi_init(&device->spi);

    /* Send a message on SPI bus. */
    return (spi_message(&device->spi, &spi_msg, 1));

} /* max7219_init */

#endif /* CONFIG_MAX7219 */
