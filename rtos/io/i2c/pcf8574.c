/*
 * pcf8574.c
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
#include <pcf8574.h>

#ifdef I2C_PCF8574
/*
 * pcf8574_init
 * @device: PCF8574 device needed to be initialized.
 * @return: Success will be returned if PCF8574 device was successfully
 *  Initialized.
 * This function will initialize an PCF8574 device.
 */
int32_t pcf8574_init(PCF8574 *device)
{
    I2C_MSG i2c_msg;
    uint8_t port;

    /* Initialize I2C device. */
    i2c_init(&device->i2c);

    /* Mark all the pins as input. */
    device->out_mask = 0x0;

    /* Initialize I2C transaction. */
    i2c_msg.buffer = &port;
    i2c_msg.length = 1;
    i2c_msg.flags = I2C_MSG_WRITE;

    /* Set all the pins to configure them as input. */
    port = 0xFF;

    /* Send a message on I2C bus. */
    return (i2c_message(&device->i2c, &i2c_msg, 1));

} /* pcf8574_init */

/*
 * pcf8574_read
 * @device: PCF8574 device for which port is needed to be read.
 * @return: Current port status will be returned here.
 * This function will read and return the port for the PCF8574 device.
 */
uint8_t pcf8574_read(PCF8574 *device)
{
    I2C_MSG i2c_msg;
    uint8_t port;

    /* Initialize I2C transaction. */
    i2c_msg.buffer = &port;
    i2c_msg.length = 1;
    i2c_msg.flags = I2C_MSG_READ;

    /* Read current port status. */
    i2c_message(&device->i2c, &i2c_msg, 1);

    /* Return the current port status. */
    return (port);

} /* pcf8574_read */

/*
 * pcf8574_write
 * @device: PCF8574 device for which port is needed to be written.
 * This function will write the port with the configured data.
 */
void pcf8574_write(PCF8574 *device)
{
    I2C_MSG i2c_msg;
    uint8_t port = 0xFF;

    /* Initialize I2C transaction. */
    i2c_msg.buffer = &port;
    i2c_msg.length = 1;
    i2c_msg.flags = I2C_MSG_WRITE;

    /* Set all the bits that are configured as input. */
    port |= ((uint8_t)~(device->out_mask));

    /* Clear any pins that are being sinked. */
    port &= device->out_data;

    /* Read current port status. */
    i2c_message(&device->i2c, &i2c_msg, 1);

} /* pcf8574_write */

#endif /* I2C_PCF8574 */
