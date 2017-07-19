/*
 * i2c.c
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

#ifdef CONFIG_I2C
#include <i2c.h>

/*
 * i2c_init
 * This function will initialize an I2C device.
 */
void i2c_init(I2C_DEVICE *device)
{
    /* Do target initialization for this device. */
    device->init(device);

} /* i2c_init */

/*
 * i2c_message
 * @device: I2C device on which we need to process a I2C message.
 * @messages: I2C message array.
 * @num_messages: Number of I2C messages to process.
 * @return: Success will be returned if all I2C messages were successfully
 *  processed.
 * This function will process given I2C messages.
 */
int32_t i2c_message(I2C_DEVICE *device, I2C_MSG *messages, uint32_t num_messages)
{
    int32_t status = SUCCESS;

    /* While we have a I2C message to send. */
    while (num_messages --)
    {
        /* Process this I2C message on the target. */
        status = device->msg(device, messages);

        /* If this I2C message was successfully processed. */
        if (status == SUCCESS)
        {
            /* Get the next message to be sent. */
            messages++;
        }
        else
        {
            /* Break and stop processing I2C messages. */
            break;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* i2c_message */

#endif /* CONFIG_I2C */
