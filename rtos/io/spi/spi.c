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
 * spi_message
 * @device: SPI device on which we need to process a SPI message.
 * @messages: SPI message array.
 * @num_messages: Number of SPI messages to process.
 * @return: Total number of bytes read or/and written.
 * This function will process given SPI messages.
 */
int32_t spi_message(SPI_DEVICE *device, SPI_MSG *messages, uint32_t num_messages)
{
    int32_t ret_bytes = 0, nbytes;

    /* Select slave for the device. */
    SPI_TGT_SS(device);

    /* While we have a SPI message to send. */
    while (num_messages --)
    {
        /* Process this SPI message on the target. */
        nbytes = SPI_TGT_MSG(device, messages);

        /* If this SPI message was successfully processed. */
        if (nbytes >= 0)
        {
            /* Add bytes to total number of bytes processed. */
            ret_bytes += nbytes;

            /* Get the next message to be sent. */
            messages++;
        }
        else
        {
            /* Return received error. */
            ret_bytes = nbytes;

            /* Break and stop processing SPI messages. */
            break;
        }
    }

    /* Un-select the SPI device. */
    SPI_TGT_SUS(device);

    /* Return number of bytes written to and read from the SPI. */
    return (ret_bytes);

} /* spi_message */

#endif /* CONFIG_SPI */
