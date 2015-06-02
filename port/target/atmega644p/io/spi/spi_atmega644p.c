/*
 * spi_atmega644p.c
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
#include <spi_atmega644p.h>

/*
 * spi_atmega644_init
 * @device: SPI device needed to be initialized.
 * This function will initialize a ATMEGA644P SPI device instance.
 */
void spi_atmega644_init(SPI_DEVICE *device)
{
    uint32_t baud_scale;
    uint8_t bit_count = 0;

    /* SPI baudrate table.
     * Divisor  SP1     SP0     SPX
     * 2        0       0       1
     * 4        0       0       0
     * 8        0       1       1
     * 16       0       1       0
     * 32       1       0       1
     * 64       1       0       0
     * 128      1       1       0
     * 64       1       1       1
     */

    /* Calculate the required baudrate prescaler. */
    baud_scale = PCLK_FREQ / device->baudrate;
    if (baud_scale >= 128)
    {
        /* Use baud scale 6. */
        baud_scale = 6;
    }
    else if (baud_scale <= 2)
    {
        /* Use baud scale 0. */
        baud_scale = 0;
    }
    else
    {
        /* Calculate the number of bits in the scale. */
        while (baud_scale)
        {
            baud_scale = baud_scale >> 1;
            bit_count++;
        }

        /* Calculate the required baud scale. */
        baud_scale = (uint32_t)(bit_count - 2);
    }

    /* Update SPCR. */
    SPCR = (((device->cfg_flags & SPI_CFG_LSB_FIRST) != 0) << ATMEGA644P_SPI_SPCR_DORD_SHIFT) |
           (((device->cfg_flags & SPI_CFG_MASTER) != 0) << ATMEGA644P_SPI_SPCR_MSTR_SHIFT) |
           ((uint32_t)((device->cfg_flags & SPI_CFG_CLK_IDLE_HIGH) != 0) << ATMEGA644P_SPI_SPCR_CPOL_SHIFT) |
           ((uint32_t)((device->cfg_flags & SPI_CFG_CLK_FIRST_DATA) == 0) << ATMEGA644P_SPI_SPCR_CPHA_SHIFT) |
           ((baud_scale & 0x06) << 1);

    /* Update SPCR. */
    SPSR = (((baud_scale & 0x01) == 0) << ATMEGA644P_SPI_SPSR_SPI2X);

    /* PIN configurations for SPI is
     *  SS - PB4
     *  MOSI - PB5
     *  MISO - PB6
     *  SCLK - PB7
     */

    /* If we are operating in master mode. */
    if (device->cfg_flags & SPI_CFG_MASTER)
    {
        /* MOSI, SCLK and SS will be output. */
        DDRB |= ((1 << 4) | (1 << 5) | (1 << 7));

        /* Set SS high. */
        PORTB |= (1 << 4);

        /* Clear MOSI and SCLK. */
        PORTB &= (uint8_t)~((1 << 5) | (1 << 7));

        /* MISO will be input. */
        DDRB &= (uint8_t)~(1 << 6);
    }
    else
    {
        /* MOSI, SCLK and SS will be input. */
        DDRB &= (uint8_t)~((1 << 4) | (1 << 5) | (1 << 7));

        /* MISO will be output. */
        DDRB |= (1 << 6);

        /* Set MISO low. */
        PORTB &= (uint8_t)~(1 << 6);
    }

    /* Enable SPI device. */
    SPCR |= (1 << ATMEGA644P_SPI_SPCR_SPE_SHIFT);

} /* spi_atmega644_init */

/*
 * spi_atmega644_slave_select
 * This function will enable the slave so it can receive data.
 */
void spi_atmega644_slave_select(SPI_DEVICE *device)
{
    /* Set SS low. */
    PORTB &= (uint8_t)~(1 << 4);

} /* spi_atmega644_slave_select */

/*
 * spi_atmega644_slave_unselect
 * This function will disable the slave.
 */
void spi_atmega644_slave_unselect(SPI_DEVICE *device)
{
    /* Set SS high. */
    PORTB |= (1 << 4);

} /* spi_atmega644_slave_unselect */

/*
 * spi_atmega644_message
 * @device: SPI device for which messages are needed to be processed.
 * @message: SPI message needed to be sent.
 * @return: Total number of bytes read or written.
 * This function will process a SPI message.
 */
int32_t spi_atmega644_message(SPI_DEVICE *device, SPI_MSG *message)
{
    int32_t num_bytes = message->length;
    uint8_t *buffer = message->buffer;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(device);

    /* While we have a byte to write and read. */
    while (num_bytes --)
    {
        /* Send a byte. */
        SPDR = *buffer;

        /* Wait for transmission to complete. */
        while(!(SPSR & (1 << SPIF)));

        /* Check if we are also reading. */
        if (message->flags & SPI_MSG_READ)
        {
            /* Save the byte read from SPI. */
            *buffer = SPDR;
        }

        /* Get next byte to send and update. */
        buffer++;
    }

    /* Return number of bytes written and read from SPI. */
    return (message->length);

} /* spi_atmega644_message */

#endif /* CONFIG_SPI */