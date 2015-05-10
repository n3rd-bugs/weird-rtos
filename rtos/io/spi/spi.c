/*
 * spi.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>

#ifdef CONFIG_SPI
#include <spi.h>

/*
 * spi_init
 * This function will initialize an SPI device.
 */
void spi_init(SPI_DEVICE *device)
{
    /* Do target initialization for this device. */
    SPI_TGT_INIT(device);

} /* spi_init */

/*
 * spi_write_read
 * @device: SPI device on which we need to write and then read data.
 * @buffer: Buffer from which data will be written, same buffer will be updated
 *  with the data written.
 * @length: Size of the provided buffer.
 * This function will write and then read data from a SPI device.
 */
int32_t spi_write_read(SPI_DEVICE *device, uint8_t *buffer, int32_t length)
{
    int32_t ret_bytes;

    /* Select slave for the device. */
    SPI_TGT_SS(device);

    /* While we have a byte to read. */
    ret_bytes = SPI_TGT_WR(device, buffer, length);

    /* Un-select the SPI device. */
    SPI_TGT_SUS(device);

    /* Return number of bytes written to and read from the SPI. */
    return (ret_bytes);

} /* spi_write_read */

#endif /* CONFIG_SPI */
