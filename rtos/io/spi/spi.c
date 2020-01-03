/*
 * spi.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

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

    /* If software slave selection is needed. */
    if (!(device->cfg_flags & SPI_CFG_ENABLE_HARD_SS))
    {
        /* Select slave for the device. */
        device->slave_select(device);
    }

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

    /* If software slave selection is needed. */
    if (!(device->cfg_flags & SPI_CFG_ENABLE_HARD_SS))
    {
        /* Un-select the SPI device. */
        device->slave_unselect(device);
    }

    /* Return status to the caller. */
    return (status);

} /* spi_message */

#endif /* CONFIG_SPI */
