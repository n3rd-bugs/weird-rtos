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
    device->init(device);

} /* spi_init */

/*
 * spi_message
 * @device: SPI device on which we need to process a SPI message.
 * @messages: SPI message array.
 * @num_messages: Number of SPI messages to process.
 * @return: Success will be returned if all SPI messages were successfully
 *  processed.
 * This function will process given SPI messages.
 */
int32_t spi_message(SPI_DEVICE *device, SPI_MSG *messages, uint32_t num_messages)
{
    int32_t status = SUCCESS;

    /* Select slave for the device. */
    device->slave_select(device);

    /* While we have a SPI message to send. */
    while (num_messages --)
    {
        /* Process this SPI message on the target. */
        status = device->msg(device, messages);

        /* If this SPI message was successfully processed. */
        if (status == SUCCESS)
        {
            /* Get the next message to be sent. */
            messages++;
        }
        else
        {
            /* Break and stop processing SPI messages. */
            break;
        }
    }

    /* Un-select the SPI device. */
    device->slave_unselect(device);

    /* Return status to the caller. */
    return (status);

} /* spi_message */

#endif /* CONFIG_SPI */
