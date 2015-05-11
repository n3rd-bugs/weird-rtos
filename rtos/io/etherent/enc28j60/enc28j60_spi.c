/*
 * enc28j60_spi.c
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

#ifdef CONFIG_ENC28J60
#include <enc28j60.h>
#include <enc28j60_spi.h>

/*
 * enc28j60_write_read_op
 * @device: ENC28J60 device instance for which write/read operation is needed
 *  to be performed.
 * @opcode: Operation needed to be performed.
 * @address: Address at which data is needed to be written.
 * @value: Value needed to be written.
 * @ret_value: If not null data read from SPI will be returned here.
 * @return: A success status will be returned if a register was successfully
 *  written, ENC28J60_SPI_ERROR will be returned if an error occurred while
 *  writing on SPI device.
 * This function will perform a write and read operation on the ENC28J60 device.
 */
int32_t enc28j60_write_read_op(ENC28J60 *device, uint8_t opcode, uint8_t address, uint8_t value, uint8_t *ret_value)
{
    int32_t status = SUCCESS;
    uint8_t data[2];

    /* Check if we need to switch memory bank. */
    if ((address & ENC28J60_BANKED_MASK) && (((address & ENC28J60_BANK_MASK) >> ENC28J60_BANK_SHIFT) != device->mem_block))
    {
        /* Clear the memory bank bits. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_ECON, (ENC28J60_ECON1_BSEL1|ENC28J60_ECON1_BSEL0), NULL) != SUCCESS)

        /* Select the required memory bank. */
        device->mem_block = ((address & ENC28J60_BANK_MASK) >> ENC28J60_BANK_SHIFT);

        /* Set the required memory bank bits. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON, device->mem_block, NULL) != SUCCESS);
    }

    /* Initialize the data needed to be sent. */
    data[0] = (uint8_t)(opcode | (address & ENC28J60_ADDR_MASK));
    data[1] = value;

    /* Perform a SPI write read operation. */
    if (spi_write_read(&device->spi, data, 2) != 2)
    {
        /* Return error to the caller. */
        status = ENC28J60_SPI_ERROR;
    }
    else
    {
        /* If we need to return the read data to the caller. */
        if (ret_value != NULL)
        {
            /* Return read data to the caller. */
            *ret_value = data[1];
        }
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_write_read_byte */

#endif /* CONFIG_ENC28J60 */
