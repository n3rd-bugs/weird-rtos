/*
 * enc28j60_spi.c
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
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>
#include <enc28j60_spi.h>

/* Internal function prototypes. */
static int32_t enc28j60_phy_wait(ENC28J60 *);

/*
 * enc28j60_phy_wait
 * @device: ENC28J60 device instance for which we need to wait for PHY
 *  operation.
 * @return: A success status will be returned if PHY operation was successfully
 *  completed, ENC28J60_SPI_ERROR will be returned if an error occurred while
 *  writing on SPI device.
 * This function will perform wait for a PHY operation to be completed on the
 * given enc28j60 device.
 */
static int32_t enc28j60_phy_wait(ENC28J60 *device)
{
    int32_t status;
    uint8_t mistat;

    /* Wait for PHY register to be written. */
    do
    {
        /* Read the value of MISTAT. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_READ_BUFFER, ENC28J60_ADDR_MISTAT, 0xFF, &mistat, 1);

    } while ((status == SUCCESS) && (mistat & ENC28J60_MISTAT_BUSY));

    /* Return status to the caller. */
    return (status);

} /* enc28j60_phy_wait */


/*
 * enc28j60_write_phy
 * @device: ENC28J60 device instance for which a PHY register is needed to be
 *  written.
 * @address: Address at which data is needed to be written.
 * @value: Value needed to be written.
 * @return: A success status will be returned if a register was successfully
 *  written, ENC28J60_SPI_ERROR will be returned if an error occurred while
 *  writing on SPI device.
 * This function will perform write a PHY register on given enc28j60 device.
 */
int32_t enc28j60_write_phy(ENC28J60 *device, uint8_t address, uint16_t value)
{
    int32_t status;

    /* Write the PHY address needed to be written on MIREGADR. */
    status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MIREGADR, address, NULL, 0);

    if (status == SUCCESS)
    {
        /* Write the data on MIWRL/MIWRH. */
        status = enc28j60_write_word(device, ENC28J60_ADDR_MIWRL, value);
    }

    if (status == SUCCESS)
    {
        /* Wait for the completion of PHY operation. */
        status = enc28j60_phy_wait(device);
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_write_phy */

/*
 * enc28j60_read_phy
 * @device: ENC28J60 device instance for which a PHY register is needed to be
 *  read.
 * @address: Address from which data is needed to be read.
 * @value: If not null read data will be returned here.
 * @return: A success status will be returned if a register was successfully
 *  read, ENC28J60_SPI_ERROR will be returned if an error occurred while
 *  reading from SPI device.
 * This function will read a PHY register from given ENC28J60 device.
 */
int32_t enc28j60_read_phy(ENC28J60 *device, uint8_t address, uint16_t *value)
{
    int32_t status;

    /* Set the PHY address needed to be read at MIREGADR. */
    status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MIREGADR, address, NULL, 0);

    if (status == SUCCESS)
    {
        /* Start the PHY read operation. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MICMD, ENC28J60_MICMD_MIIRD, NULL, 0);
    }

    if (status == SUCCESS)
    {
        /* Wait for the completion of PHY operation. */
        status = enc28j60_phy_wait(device);
    }

    if (status == SUCCESS)
    {
        /* Stop the PHY read operation. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MICMD, 0x00, NULL, 0);
    }

    if (status == SUCCESS)
    {
        /* Read and return the PHY register. */
        status = enc28j60_read_word(device, ENC28J60_ADDR_MIRDL, value);
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_read_phy */

/*
 * enc28j60_write_buffer
 * @device: ENC28J60 device instance for which buffer memory is needed to be
 *  written.
 * @address: Address at which given data is needed to be written.
 * @value: Data needed to be written.
 * @length: Number of bytes needed to be written.
 * @return: A success status will be returned if buffer memory was successfully
 *  written, ENC28J60_SPI_ERROR will be returned if an error occurred while
 *  writing on SPI device.
 * This function will write buffer memory from a enc28j60 device.
 */
int32_t enc28j60_write_buffer(ENC28J60 *device, uint16_t address, uint8_t *value, int32_t length)
{
    int32_t status;

    /* Set the buffer address to which we will be writing data. */
    status = enc28j60_write_word(device, ENC28J60_ADDR_EWRPTL, address);

    if (status == SUCCESS)
    {
        /* Write the required buffer memory. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_BUFFER, ENC28J60_ADDR_BUFFER, 0xFF, value, length);
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_write_buffer */

/*
 * enc28j60_read_buffer
 * @device: ENC28J60 device instance for which buffer memory is needed to be
 *  read.
 * @address: Address from which data is needed to be read.
 * @value: If not null read data will be returned here.
 * @length: Number of bytes needed to be read.
 * @return: A success status will be returned if buffer memory was successfully
 *  read, ENC28J60_SPI_ERROR will be returned if an error occurred while
 *  reading from SPI device.
 * This function will read a buffer memory from a enc28j60 device.
 */
int32_t enc28j60_read_buffer(ENC28J60 *device, uint16_t address, uint8_t *value, int32_t length)
{
    int32_t status;

    /* Set the buffer address from which we need to read buffer. */
    status = enc28j60_write_word(device, ENC28J60_ADDR_ERDPTL, address);

    if (status == SUCCESS)
    {
        /* Read the buffer memory. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_READ_BUFFER, ENC28J60_ADDR_BUFFER, 0xFF, value, length);
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_read_buffer */

/*
 * enc28j60_write_word
 * @device: ENC28J60 device instance for which data is needed to be written.
 * @address: Address at which data is needed to be written.
 * @value: Value needed to be written.
 * @return: A success status will be returned if a register was successfully
 *  written, ENC28J60_SPI_ERROR will be returned if an error occurred while
 *  writing on SPI device.
 * This function will perform write a word (2-bytes) on the given address of
 * enc28j60 device.
 */
int32_t enc28j60_write_word(ENC28J60 *device, uint8_t address, uint16_t value)
{
    int32_t status;

    /* Write the first byte. */
    status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, (address), (uint8_t)(value & 0xFF), NULL, 0);

    if (status == SUCCESS)
    {
        /* Write the second byte. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, (uint8_t)(address + 1), (uint8_t)(value >> 8), NULL, 0);
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_write_word */

/*
 * enc28j60_read_word
 * @device: ENC28J60 device instance for which data is needed to be read.
 * @address: Address from which data is needed to be read.
 * @value: If not null read value will be returned here.
 * @return: A success status will be returned if a register was successfully
 *  read, ENC28J60_SPI_ERROR will be returned if an error occurred while
 *  reading from SPI device.
 * This function will read a word (2-bytes) from the given address of
 * enc28j60 device.
 */
int32_t enc28j60_read_word(ENC28J60 *device, uint8_t address, uint16_t *value)
{
    int32_t status;
    uint8_t first_byte, second_byte;

    /* Read the first byte. */
    status = enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, address, 0xFF, &first_byte, 1);

    if (status == SUCCESS)
    {
        /* Read the second byte. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, (uint8_t)(address + 1), 0xFF, &second_byte, 1);
    }

    if ((status == SUCCESS) && (value != NULL))
    {
        /* Return the read word. */
        *value = (uint16_t)((second_byte << 8) | first_byte);
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_read_word */

/*
 * enc28j60_write_read_op
 * @device: ENC28J60 device instance for which write/read operation is needed
 *  to be performed.
 * @opcode: Operation needed to be performed.
 * @address: Address at which data is needed to be written.
 * @value: Value needed to be written.
 *  ENC28J60_OP_READ_CTRL: This is unused.
 *  ENC28J60_OP_READ_BUFFER: This is unused.
 *  ENC28J60_OP_WRITE_CTRL: Control register value needed to be written.
 *  ENC28J60_OP_WRITE_BUFFER: This is unused.
 *  ENC28J60_OP_BIT_SET: Bit address to be set.
 *  ENC28J60_OP_BIT_CLR: Bit address to be cleared.
 *  ENC28J60_OP_RESET: Should be reset address i.e. 0x1F.
 * @rx_value: If not null a buffered value will be given here.
 *  ENC28J60_OP_READ_CTRL: Buffer in which data will be returned.
 *  ENC28J60_OP_READ_BUFFER: Buffer in which data will be returned.
 *  ENC28J60_OP_WRITE_CTRL: Should be null.
 *  ENC28J60_OP_WRITE_BUFFER: Buffer needed to be written.
 *  ENC28J60_OP_BIT_SET: Should be null.
 *  ENC28J60_OP_BIT_CLR: Should be null.
 *  ENC28J60_OP_RESET: Should be null.
 * @rx_length: Number of bytes needed to be read from SPI.
 * @return: A success status will be returned if a register was successfully
 *  written, ENC28J60_SPI_ERROR will be returned if an error occurred while
 *  writing on SPI device.
 * This function will perform a write and read operation on the ENC28J60 device.
 */
int32_t enc28j60_write_read_op(ENC28J60 *device, uint8_t opcode, uint8_t address, uint8_t value, uint8_t *buffer, int32_t length)
{
    int32_t status = SUCCESS;
    uint8_t op_buffer[2];
    uint8_t rx_byte = value;
    SPI_MSG msg[2];

    /* Check if we need to switch memory bank. */
    if (((address & ENC28J60_ADDR_MASK) < ENC28J60_NON_BANKED) && (((address & ENC28J60_BANK_MASK) >> ENC28J60_BANK_SHIFT) != device->mem_block))
    {
        /* Clear the memory bank bits. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_ECON1, (ENC28J60_ECON1_BSEL1|ENC28J60_ECON1_BSEL0), NULL, 0);

        if (status == SUCCESS)
        {
            /* Select the required memory bank. */
            device->mem_block = ((address & ENC28J60_BANK_MASK) >> ENC28J60_BANK_SHIFT);

            /* Set the required memory bank bits. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, device->mem_block, NULL, 0);
        }
    }

    if (status == SUCCESS)
    {
        /* Initialize first SPI message. */
        msg[0].buffer = op_buffer;
        msg[0].length = 1;
        msg[0].flags = SPI_MSG_WRITE;

        /* Initialize the data needed to be sent. */
        op_buffer[0] = (uint8_t)(opcode | (address & ENC28J60_ADDR_MASK));

        /* If MAC or MII register is being read. */
        if ((opcode == ENC28J60_OP_READ_CTRL) && (address & ENC28J60_MAC_MII_MASK))
        {
            /* Actual data will come after a dummy byte. */
            msg[0].length++;
        }

        /* Initialize second SPI message. */

        /* If we have a buffer in which we will be returning data. */
        if (buffer != NULL)
        {
            /* Save data in the given buffer. */
            msg[1].buffer = buffer;

            /* Read the required number of bytes. */
            msg[1].length = length;

            /* If we are performing a read operation. */
            if ((opcode == ENC28J60_OP_READ_CTRL) || (opcode == ENC28J60_OP_READ_BUFFER))
            {
                /* This will be a read message. */
                msg[1].flags = SPI_MSG_READ;
            }
            else
            {
                /* This will be a write message. */
                msg[1].flags = SPI_MSG_WRITE;
            }
        }
        else
        {
            /* We might be reading or writing a byte so just perform both of them. */
            msg[1].flags = (SPI_MSG_WRITE | SPI_MSG_READ);

            /* Save data in a data buffer. */
            msg[1].buffer = &rx_byte;

            /* Only read one byte. */
            msg[1].length = 1;
        }

        /* Transfer these SPI messages. */
        status = spi_message(&device->spi, msg, 2);
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_write_read_byte */

#endif /* ETHERNET_ENC28J60 */
