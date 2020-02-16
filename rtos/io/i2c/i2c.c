/*
 * i2c.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef IO_I2C
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

#endif /* IO_I2C */
