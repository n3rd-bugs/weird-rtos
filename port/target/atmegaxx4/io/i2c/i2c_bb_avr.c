/*
 * i2c_bb_avr.c
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

#ifdef CONFIG_I2C
#include <i2c_bb_avr.h>
#include <util/delay.h>

/*
 * i2c_bb_avr_init
 * @device: I2C device needed to be initialized.
 * This function will initialize a bit-bang I2C device instance.
 */
void i2c_bb_avr_init(I2C_DEVICE *device)
{
    I2C_BB_AVR *bb_avr = (I2C_BB_AVR *)device->data;
    INT_LVL interrupt_level;

    /* Get system interrupt level. */
    interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Set SCL and SDA high. */
    _SFR_IO8(bb_avr->port_scl) |= (1 << bb_avr->pin_num_scl);
    _SFR_IO8(bb_avr->port_sda) |= (1 << bb_avr->pin_num_sda);

    /* SCL and SDA will be output. */
    _SFR_IO8(bb_avr->ddr_scl) |= (1 << bb_avr->pin_num_scl);
    _SFR_IO8(bb_avr->ddr_sda) |= (1 << bb_avr->pin_num_sda);

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

} /* i2c_bb_avr_init */

/*
 * i2c_bb_avr_message
 * @device: I2C device for which messages are needed to be processed.
 * @message: I2C message needed to be sent.
 * @return: Success will be returned if I2C message was successfully processed,
 *  I2C_MSG_NACKED will be returned if a byte was NACKed.
 * This function will process a I2C message.
 */
int32_t i2c_bb_avr_message(I2C_DEVICE *device, I2C_MSG *message)
{
    I2C_BB_AVR *bb_avr = (I2C_BB_AVR *)device->data;
    int32_t j, status = SUCCESS;
    uint8_t *ptr, acked = TRUE, byte_tx, byte_rx, i, k, this_len, port_sda, pin_map_sda, ddr_sda, pin_sda, port_scl, pin_map_scl;
    INT_LVL interrupt_level;

    /* Save the register addresses we will need to access. */
    port_sda = bb_avr->port_sda;
    ddr_sda = bb_avr->ddr_sda;
    pin_sda = bb_avr->pin_sda;
    pin_map_sda = (1 << bb_avr->pin_num_sda);
    port_scl = bb_avr->port_scl;
    pin_map_scl = (1 << bb_avr->pin_num_scl);

    /* Get system interrupt level. */
    interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Send start bit. */
    /* Clear SDA and SCL. */
    _SFR_IO8(port_sda) &= (uint8_t)(~(pin_map_sda));
    _SFR_IO8(port_scl) &= (uint8_t)(~(pin_map_scl));

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Write first byte of the message. */
    byte_tx = ((device->address << 1) | (message->flags == I2C_MSG_READ));

    /* Get system interrupt level. */
    interrupt_level = GET_INTERRUPT_LEVEL();

    /* Transfer a byte bit by bit. */
    for (i = 0; i < 8; i++)
    {
        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* If we need to send a high. */
        if ((byte_tx & 0x80) != 0)
        {
            /* Set SDA. */
            _SFR_IO8(port_sda) |= pin_map_sda;
        }
        else
        {
            /* Clear SDA. */
            _SFR_IO8(port_sda) &= (uint8_t)(~(pin_map_sda));
        }

        /* Toggle SCL. */
        _SFR_IO8(port_scl) ^= pin_map_scl;

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* A bit is now transfered */
        byte_tx = (byte_tx << 1);

#ifdef AVR_SLOW_I2C
        /* Insert a delay. */
        _delay_us(AVR_I2C_DELAY);
#endif /* AVR_SLOW_I2C */

        /* Disable global interrupts. */
        DISABLE_INTERRUPTS();

        /* Toggle SCL. */
        _SFR_IO8(port_scl) ^= pin_map_scl;

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);
    }

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Make SDA input. */
    _SFR_IO8(ddr_sda) &= (uint8_t)(~(pin_map_sda));

    /* Toggle SCL. */
    _SFR_IO8(port_scl) ^= pin_map_scl;

    /* See if remote has not acknowledged the byte. */
    if ((_SFR_IO8(pin_sda) & pin_map_sda) != 0)
    {
        /* Return error to the caller. */
        status = I2C_MSG_NACKED;
    }

    /* Toggle SCL. */
    _SFR_IO8(port_scl) ^= pin_map_scl;

    /* Make SDA output. */
    _SFR_IO8(ddr_sda) |= pin_map_sda;

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Save the message start pointer. */
    ptr = message->buffer;

    if (status == SUCCESS)
    {
        /* Process the message request. */
        switch (message->flags & (I2C_MSG_WRITE | I2C_MSG_READ))
        {

        /* If we are writing. */
        case I2C_MSG_WRITE:

            /* While we have a byte to write. */
            for (j = 0; ((acked == TRUE) && (j < message->length)); j += this_len)
            {
                /* See if we need to transfer more than 255 bytes. */
                if ((message->length - j) > 255)
                {
                    /* Lets transfer 255 bytes. */
                    this_len = 255;
                }
                else
                {
                    /* Send transfer remaining number of bytes. */
                    this_len = (message->length - j);
                }

                /* Transfer this chuck. */
                for (k = 0; ((acked == TRUE) && (k < this_len)); k++)
                {
                    /* Save the byte we need to send. */
                    byte_tx = *ptr;

                    /* Transfer a byte bit by bit. */
                    for (i = 0; i < 8; i++)
                    {
                        /* Disable global interrupts. */
                        DISABLE_INTERRUPTS();

                        /* If we need to send a high. */
                        if ((byte_tx & 0x80) != 0)
                        {
                            /* Set SDA. */
                            _SFR_IO8(port_sda) |= pin_map_sda;
                        }
                        else
                        {
                            /* Clear SDA. */
                            _SFR_IO8(port_sda) &= (uint8_t)(~(pin_map_sda));
                        }

                        /* Toggle SCL. */
                        _SFR_IO8(port_scl) ^= pin_map_scl;

                        /* Restore old interrupt level. */
                        SET_INTERRUPT_LEVEL(interrupt_level);

                        /* A bit is now transfered */
                        byte_tx = (byte_tx << 1);

#ifdef AVR_SLOW_I2C
                        /* Insert a delay. */
                        _delay_us(AVR_I2C_DELAY);
#endif /* AVR_SLOW_I2C */

                        /* Disable global interrupts. */
                        DISABLE_INTERRUPTS();

                        /* Toggle SCL. */
                        _SFR_IO8(port_scl) ^= pin_map_scl;

                        /* Restore old interrupt level. */
                        SET_INTERRUPT_LEVEL(interrupt_level);
                    }

                    /* Disable global interrupts. */
                    DISABLE_INTERRUPTS();

                    /* Make SDA input. */
                    _SFR_IO8(ddr_sda) &= (uint8_t)(~(pin_map_sda));

                    /* Toggle SCL. */
                    _SFR_IO8(port_scl) ^= pin_map_scl;

                    /* See if remote has not acknowledged the byte
                     * i.e. SDA is high. */
                    if ((_SFR_IO8(pin_sda) & pin_map_sda) != 0)
                    {
                        acked = FALSE;
                    }

                    /* Get next byte to send and update. */
                    ptr++;

                    /* Toggle SCL. */
                    _SFR_IO8(port_scl) ^= pin_map_scl;

                    /* Make SDA output. */
                    _SFR_IO8(ddr_sda) |= pin_map_sda;

                    /* Restore old interrupt level. */
                    SET_INTERRUPT_LEVEL(interrupt_level);
                }
            }

            /* If remote NACKed a byte. */
            if (acked == FALSE)
            {
                /* Return error to the caller. */
                status = I2C_MSG_NACKED;
            }

            break;

        /* If we are reading. */
        case I2C_MSG_READ:

            /* Disable global interrupts. */
            DISABLE_INTERRUPTS();

            /* Make SDA input. */
            _SFR_IO8(ddr_sda) &= (uint8_t)(~(pin_map_sda));

            /* Restore old interrupt level. */
            SET_INTERRUPT_LEVEL(interrupt_level);

            /* While we have a byte to read. */
            for (j = 0; j < message->length; j += this_len)
            {
                /* See if we need to transfer more than 255 bytes. */
                if ((message->length - j) > 255)
                {
                    /* Lets transfer 255 bytes. */
                    this_len = 255;
                }
                else
                {
                    /* Send transfer remaining number of bytes. */
                    this_len = (message->length - j);

                    /* This will be the last chunk. */
                    acked = FALSE;
                }

                /* Transfer this chuck. */
                for (k = 0; k < this_len; k++)
                {
                    /* Initialize the RX byte. */
                    byte_rx = 0x00;

                    /* Transfer a byte bit by bit. */
                    for (i = 0; i < 8; i++)
                    {
                        /* Make space in the RX byte for this bit. */
                        byte_rx = (byte_rx << 1);

                        /* Disable global interrupts. */
                        DISABLE_INTERRUPTS();

                        /* Toggle SCL. */
                        _SFR_IO8(port_scl) ^= pin_map_scl;

                        /* Restore old interrupt level. */
                        SET_INTERRUPT_LEVEL(interrupt_level);

#ifdef AVR_SLOW_I2C
                        /* Insert a delay. */
                        _delay_us(AVR_I2C_DELAY);
#endif /* AVR_SLOW_I2C */

                        /* Disable global interrupts. */
                        DISABLE_INTERRUPTS();

                        /* If SDA is high. */
                        if ((_SFR_IO8(pin_sda) & pin_map_sda) != 0)
                        {
                            /* Set this bit in RX register. */
                            byte_rx |= 0x01;
                        }

                        /* Toggle SCL. */
                        _SFR_IO8(port_scl) ^= pin_map_scl;

                        /* Restore old interrupt level. */
                        SET_INTERRUPT_LEVEL(interrupt_level);

#ifdef AVR_SLOW_I2C
                        /* Insert a delay. */
                        _delay_us(AVR_I2C_DELAY);
#endif /* AVR_SLOW_I2C */
                    }

                    /* Disable global interrupts. */
                    DISABLE_INTERRUPTS();

                    /* Make SDA output. */
                    _SFR_IO8(ddr_sda) |= pin_map_sda;

                    /* ACK this byte if we have to. */
                    if (acked == TRUE)
                    {
                        /* Set SDA low to ACK the byte. */
                        _SFR_IO8(port_sda) &= (uint8_t)(~(pin_map_sda));
                    }

                    /* If we are receiving the last byte. */
                    else if ((k + 1) == this_len)
                    {
                        /* Set SDA high to NACK the byte. */
                        _SFR_IO8(port_sda) |= pin_map_sda;
                    }

                    /* Toggle SCL. */
                    _SFR_IO8(port_scl) ^= pin_map_scl;

                    /* Save the byte read from I2C. */
                    *ptr = byte_rx;

                    /* Get next byte to send and update. */
                    ptr++;

                    /* Toggle SCL. */
                    _SFR_IO8(port_scl) ^= pin_map_scl;

                    /* Make SDA input. */
                    _SFR_IO8(ddr_sda) &= (uint8_t)(~(pin_map_sda));

                    /* Restore old interrupt level. */
                    SET_INTERRUPT_LEVEL(interrupt_level);
                }
            }

            /* Disable global interrupts. */
            DISABLE_INTERRUPTS();

            /* Make SDA output. */
            _SFR_IO8(ddr_sda) |= pin_map_sda;

            /* Restore old interrupt level. */
            SET_INTERRUPT_LEVEL(interrupt_level);

            break;
        }
    }

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Send stop bit. */
    /* Clear SDA. */
    _SFR_IO8(port_sda) &= (uint8_t)(~(pin_map_sda));

    /* Set SCL and SDA. */
    _SFR_IO8(port_scl) |= (pin_map_scl);
    _SFR_IO8(port_sda) |= (pin_map_sda);

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Return status to the caller. */
    return (status);

} /* i2c_bb_avr_message */

#endif /* CONFIG_I2C */
