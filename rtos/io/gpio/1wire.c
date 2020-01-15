/*
 * 1wire.c
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
#include <1wire.h>

#ifdef CONFIG_1WIRE
/*
 * onewire_init
 * @onewire: 1-wire bus needed to be initialized.
 * @return: Always return success.
 * This function will initialize a 1-wire bus .
 */
int32_t onewire_init(ONE_WIRE *onewire)
{
    /* Initialize data pin. */
    onewire->pin_init(onewire);

    /* Always return success. */
    return (SUCCESS);

} /* onewire_init */

/*
 * onewire_reset
 * @onewire: 1-wire bus needed to be reset.
 * @return: Success will be returned if bus was successfully reset,
 *  ONEWIRE_TIMEOUT will be returned if timeout occurred while waiting for bus,
 *  ONEWIRE_NO_DEVICE will be returned if no device responded with presence pulse.
 * This function will reset the given 1-wire bus.
 */
int32_t onewire_reset(ONE_WIRE *onewire)
{
    int32_t status;
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();
    uint16_t delta;

    /* Reset the status. */
    status = SUCCESS;

    /* Set the pin as input. */
    onewire->set_pin_mode(onewire, FALSE);

    /* Wait for the 1-wire bus to free. */
    POLL_HW_US((!onewire->get_pin_state(onewire)), ONEWIRE_INIT_HIGH_DELAY, status, ONEWIRE_TIMEOUT);

    if (status == SUCCESS)
    {
        /* Disable interrupts. */
        DISABLE_INTERRUPTS();

        /* Pull down the line. */
        onewire->set_pin_mode(onewire, TRUE);
        onewire->set_pin_state(onewire, FALSE);

        /* Wait for reset pulse to complete. */
        sleep_us(ONEWIRE_RESET_DELAY);

        /* Set the pin as input. */
        onewire->set_pin_mode(onewire, FALSE);

        /* Wait for the bus to pull down the line. */
        POLL_HW_US_D((onewire->get_pin_state(onewire)), ONEWIRE_RESET_DELAY, delta, status, ONEWIRE_TIMEOUT);

        /* Restore interrupts. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        if (status == SUCCESS)
        {
            /* See if a device responded in time. */
            if (HW_TICK_TO_US(delta) < ONEWIRE_PRESENCE_DELAY)
            {
                /* Wait for bus to free. */
                POLL_HW_US(!(onewire->get_pin_state(onewire)), ONEWIRE_RESET_DELAY, status, ONEWIRE_TIMEOUT);
            }
            else
            {
                /* No device found on the bus. */
                status = ONEWIRE_NO_DEVICE;
            }
        }
        else
        {
            /* No device found on the bus. */
            status = ONEWIRE_NO_DEVICE;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* onewire_reset */

/*
 * onewire_select
 * @onewire: 1-wire bus on which a ROM is needed to be selected.
 * @rom_addr: ROM needed to be selected.
 * @return: Success will be returned if requested ROM was successfully selected.
 * This function will select the given ROM on 1-wire bus.
 */
int32_t onewire_select(ONE_WIRE *onewire, const uint8_t *rom_addr)
{
    int32_t status;

    /* Send the ROM select command. */
    status = onewire_write(onewire, (uint8_t []){ONEWIRE_CMD_CHOOSE_ROM}, 1);

    if (status == SUCCESS)
    {
        /* Send the required ROM address. */
        status = onewire_write(onewire, rom_addr, 8);
    }

    /* Return status to the caller. */
    return (status);

} /* onewire_select */

/*
 * onewire_read_bit
 * @onewire: 1-wire bus needed to be read.
 * @bit: Bit read will be returned here.
 * @return: Success will be returned if bit was successfully read,
 *  ONEWIRE_TIMEOUT will be returned if timeout occurred while waiting for bus.
 * This function will read a bit from 1-wire bus.
 */
int32_t onewire_read_bit(ONE_WIRE *onewire, uint8_t *bit)
{
    int32_t status = SUCCESS;
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();
    uint16_t delta;

    /* Disable interrupts. */
    DISABLE_INTERRUPTS();

    /* Pull down the line. */
    onewire->set_pin_mode(onewire, TRUE);
    onewire->set_pin_state(onewire, FALSE);

    /* Wait for the bus to be triggered. */
    sleep_us(ONEWIRE_BUS_TRIGGER);

    /* Set the pin as input. */
    onewire->set_pin_mode(onewire, FALSE);

    /* Wait for the bus to pull up the line. */
    POLL_HW_US_D(!(onewire->get_pin_state(onewire)), ONEWIRE_BIT_DELAY, delta, status, ONEWIRE_TIMEOUT);

    /* Restore interrupts. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    if (status == SUCCESS)
    {
        /* If we read a high. */
        if (HW_TICK_TO_US(delta) < ONEWIRE_BIT_HIGH_DELAY)
        {
            /* Return high. */
            *bit = TRUE;
        }
        else
        {
            /* Return low. */
            *bit = FALSE;
        }

        /* Wait for read to complete. */
        sleep_us(ONEWIRE_BIT_DELAY - HW_TICK_TO_US(delta));
    }

    /* Return status to the caller. */
    return (status);

} /* onewire_read_bit */

/*
 * onewire_read
 * @onewire: 1-wire bus needed to be read.
 * @buffer: Buffer in which we need to read the data.
 * @num_bytes: Number of bytes to be read from 1-wire bus.
 * @return: Success will be returned if data was successfully read,
 *  ONEWIRE_TIMEOUT will be returned if timeout occurred while waiting for bus.
 * This function will read the given number of bytes from 1-wire bus.
 */
int32_t onewire_read(ONE_WIRE *onewire, uint8_t *buffer, int32_t num_bytes)
{
    int32_t i, j, status = SUCCESS;
    uint8_t bit;

    /* Read the given number of bytes. */
    for (i = 0; i < num_bytes; i++)
    {
        /* Initialize the byte with zero. */
        buffer[i] = 0x0;

        /* Read all the bits in a byte. */
        for (j = 0; (status == SUCCESS) && (j < 8); j++)
        {
            /* Read a bit from the 1-wire bus. */
            status = onewire_read_bit(onewire, &bit);

            if (status == SUCCESS)
            {
                /* If we read a high. */
                if (bit)
                {
                    /* Set this bit. */
                    buffer[i] |= (uint8_t)(1 << j);
                }
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* onewire_read */

/*
 * onewire_write_bit
 * @onewire: 1-wire bus needed to be written.
 * @bit: Bit needed to be written.
 * @return: Success will be returned if a bit was successfully written.
 * This function will write a bit on 1-wire bus.
 */
int32_t onewire_write_bit(ONE_WIRE *onewire, uint8_t bit)
{
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable interrupts. */
    DISABLE_INTERRUPTS();

    /* Pull down the line. */
    onewire->set_pin_mode(onewire, TRUE);
    onewire->set_pin_state(onewire, FALSE);

    /* Trigger the 1-wire bus. */
    sleep_us(ONEWIRE_BIT_HIGH_DELAY);

    /* If we need to send a high. */
    if (bit)
    {
        /* Pull up the line. */
        onewire->set_pin_state(onewire, TRUE);
    }

    /* Wait for read to complete. */
    sleep_us(ONEWIRE_BIT_DELAY - ONEWIRE_BIT_HIGH_DELAY);

    /* Restore interrupts. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Set the pin as input. */
    onewire->set_pin_mode(onewire, FALSE);

    /* Wait for bus to settle. */
    sleep_us(ONEWIRE_BUS_TRIGGER);

    /* Always return success. */
    return (SUCCESS);

} /* onewire_write_bit */

/*
 * onewire_write
 * @onewire: 1-wire bus needed to be written.
 * @buffer: Data buffer needed to be written.
 * @num_bytes: Number of bytes to be written.
 * @return: Success will be returned if data was successfully written.
 * This function will write the given number of bytes on 1-wire bus.
 */
int32_t onewire_write(ONE_WIRE *onewire, const uint8_t *buffer, int32_t num_bytes)
{
    int32_t i, j;

    /* Read the given number of bytes. */
    for (i = 0; i < num_bytes; i++)
    {
        /* Read all the bits in a byte. */
        for (j = 0; (j < 8); j++)
        {
            /* Write a bit on the one wire. */
            onewire_write_bit(onewire, ((buffer[i] & (1 << j)) != 0));
        }
    }

    /* Always return success. */
    return (SUCCESS);

} /* onewire_write */

/*
 * onewire_search
 * @onewire: 1-wire bus needed to be searched.
 * @search_cmd: Search command to be used.
 * @last_addr: On input this will contain the last address, on output will return the
 *  address of new device if found.
 * @return: Success will be returned if a device was found,
 *  ONEWIRE_NO_DEVICE will be returned if a device was not found.
 * This function will search the bus for available devices.
 */
int32_t onewire_search(ONE_WIRE *onewire, uint8_t search_cmd, uint8_t *last_addr)
{
    int32_t status;
    uint8_t i, j, id_bit, id_comp_bit, direction, last_zero = 0;

    /* If we need to reset our search. */
    if ((last_addr[0] == 0) && (last_addr[1] == 0) && ( last_addr[2] == 0) && ( last_addr[3] == 0) && ( last_addr[4] == 0) && ( last_addr[5] == 0) && ( last_addr[6] == 0) && ( last_addr[7] == 0))
    {
        /* Reset the last discrepancy. */
        onewire->last_discrepancy = 0;
        onewire->last_device = 0;
    }

    /* If there is a devices to be searched on the bus. */
    if (!onewire->last_device)
    {
        /* Reset the 1-wire device. */
        status = onewire_reset(onewire);
    }
    else
    {
        /* There is no next device. */
        status = ONEWIRE_NO_DEVICE;
    }

    if (status == SUCCESS)
    {
        /* Issue search command. */
        status = onewire_write(onewire, &search_cmd, 1);
    }

    /* Process all the ROM bytes [0 - 7]. */
    for (i = 0; (status == SUCCESS) && (i < 8); i++)
    {
        /* Process all the bits in a byte. */
        for (j = 0; (status == SUCCESS) && (j < 8); j++)
        {
            /* Read the ID bit. */
            status = onewire_read_bit(onewire, &id_bit);

            if (status == SUCCESS)
            {
                /* Read ID complement bit. */
                status = onewire_read_bit(onewire, &id_comp_bit);
            }

            if (status == SUCCESS)
            {
                /* Check if there is no device on the bus. */
                if ((id_bit != 1) || (id_comp_bit != 1))
                {
                    /* If need to change direction. */
                    if (id_bit != id_comp_bit)
                    {
                        /* Pick the new direction. */
                        direction = id_bit;
                    }
                    else
                    {
                        /* If we have already searched this bit. */
                        if ((i * j) < onewire->last_discrepancy)
                        {
                            /* Pick the last path. */
                            direction = (uint8_t)(last_addr[i] & (1 << j));
                        }
                        else
                        {
                            /* If equal to last. */
                            if ((i * j) == onewire->last_discrepancy)
                            {
                                /* Pick 1. */
                                direction = 1;
                            }
                            else
                            {
                                /* Pick 0. */
                                direction = 1;
                            }
                        }

                        /* If just picked a zero. */
                        if (!direction)
                        {
                            /* Save the last discrepancy. */
                            last_zero = (uint8_t)(i * j);
                        }
                    }

                    /* If need to set high. */
                    if (direction)
                    {
                        /* Set high. */
                        last_addr[i] |= (uint8_t)(1 << j);
                    }
                    else
                    {
                        /* Set low. */
                        last_addr[i] &= (uint8_t)~(1 << j);
                    }

                    /* Write this bit. */
                    status = onewire_write_bit(onewire, direction);
                }
                else
                {
                    /* No more device. */
                    status = ONEWIRE_NO_DEVICE;

                    break;
                }
            }
        }
    }

    if (status == SUCCESS)
    {
        /* Save the last discrepancy. */
        onewire->last_discrepancy = last_zero;

        /* If we did not find any deviations. */
        if (onewire->last_discrepancy == 0)
        {
            /* This must be the last device. */
            onewire->last_device = TRUE;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* onewire_search */

/*
 * onewire_crc
 * @buffer: Buffer for which CRC is needed to be calculated.
 * @num_bytes: Number of bytes in the buffer.
 * @return: Calculated CRC will be returned here.
 * This function will calculate the 1-wire CRC for the given buffer.
 */
uint8_t onewire_crc(const uint8_t *buffer, uint8_t num_bytes)
{
    uint8_t byte, mix, i, j, crc = 0;

    for (i = 0; i < num_bytes; i++)
    {
        /* Pick a byte from the buffer. */
        byte = *buffer++;

        /* Loop through each bit. */
        for (j = 0; j < 8; j++)
        {
            /* Update the mix. */
            mix = (crc ^ byte) & 0x01;

            /* Update the CRC. */
            crc >>= 1;

            /* If mix is not 0. */
            if (mix)
            {
                /* Update the CRC. */
                crc ^= 0x8C;
            }

            /* Left shift the byte. */
            byte >>= 1;
        }
    }

    /* Return the calculated CRC. */
    return (crc);

} /* onewire_crc */

#endif /* CONFIG_1WIRE */
