/*
 * spi_bb_atmega644p.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
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
#include <spi_bb_atmega644p.h>

/*
 * spi_bb_atmega644_init
 * @device: SPI device needed to be initialized.
 * This function will initialize a bit-bang SPI device instance.
 */
void spi_bb_atmega644_init(SPI_DEVICE *device)
{
    SPI_BB_AVR *bb_avr = (SPI_BB_AVR *)device->data;

    /* Only mast mode is supported. */
    OS_ASSERT((device->cfg_flags & SPI_CFG_MASTER) == 0);

    /* MOSI, SCLK and SS will be output. */
    _SFR_IO8(bb_avr->ddr_mosi) |= (1 << bb_avr->pin_num_mosi);
    _SFR_IO8(bb_avr->ddr_sclk) |= (1 << bb_avr->pin_num_sclk);
    _SFR_IO8(bb_avr->ddr_ss) |= (1 << bb_avr->pin_num_ss);

    /* Set SS high. */
    _SFR_IO8(bb_avr->port_ss) |= (1 << bb_avr->pin_num_ss);

    /* Clear MOSI and SCLK. */
    _SFR_IO8(bb_avr->port_mosi) &= (uint8_t)(~(1 << bb_avr->pin_num_mosi));
    _SFR_IO8(bb_avr->port_sclk) &= (uint8_t)(~(1 << bb_avr->pin_num_sclk));

    /* MISO will be input. */
    _SFR_IO8(bb_avr->ddr_miso) &= (uint8_t)(~(1 << bb_avr->pin_num_miso));

} /* spi_bb_atmega644_init */

/*
 * spi_bb_atmega644_slave_select
 * This function will enable the slave so it can receive data.
 */
void spi_bb_atmega644_slave_select(SPI_DEVICE *device)
{
    SPI_BB_AVR *bb_avr = (SPI_BB_AVR *)device->data;

    /* Set SS low. */
    _SFR_IO8(bb_avr->port_ss) &= (uint8_t)(~(1 << bb_avr->pin_num_ss));

} /* spi_bb_atmega644_slave_select */

/*
 * spi_bb_atmega644_slave_unselect
 * This function will disable the slave.
 */
void spi_bb_atmega644_slave_unselect(SPI_DEVICE *device)
{
    SPI_BB_AVR *bb_avr = (SPI_BB_AVR *)device->data;

    /* Set SS high. */
    _SFR_IO8(bb_avr->port_ss) |= (1 << bb_avr->pin_num_ss);

} /* spi_bb_atmega644_slave_unselect */

/*
 * spi_bb_atmega644_message
 * @device: SPI device for which messages are needed to be processed.
 * @message: SPI message needed to be sent.
 * @return: Total number of bytes read or written.
 * This function will process a SPI message.
 */
int32_t spi_bb_atmega644_message(SPI_DEVICE *device, SPI_MSG *message)
{
    SPI_BB_AVR *bb_avr = (SPI_BB_AVR *)device->data;
    int32_t j;
    uint8_t *ptr, need_read, byte_tx, byte_rx, i, port_mosi, pin_num_mosi, port_sclk, pin_num_sclk, pin_miso, pin_num_miso;

    /* Save the register addresses we will need to access. */
    port_mosi = bb_avr->port_mosi;
    pin_num_mosi = bb_avr->pin_num_mosi;
    port_sclk = bb_avr->port_sclk;
    pin_num_sclk =  bb_avr->pin_num_sclk;
    pin_miso = bb_avr->pin_miso;
    pin_num_miso =  bb_avr->pin_num_miso;

    /* Save the message start pointer. */
    ptr = message->buffer;

    /* Save if we also need to read back the data. */
    need_read = ((message->flags & SPI_MSG_READ) != 0);

    /* While we have a byte to write and read. */
    for (j = 0; j < message->length; j++)
    {
        /* Save the byte we need to send. */
        byte_tx = *ptr;
        byte_rx = 0x00;

        /* Transfer a byte bit by bit. */
        for (i = 0; i < 8; i++)
        {
            /* Make space in the RX byte for this bit. */
            byte_rx = (byte_rx << 1);

            /* If we need to send a high. */
            if ((byte_tx & (1 << 7)) != 0)
            {
                /* Set the MOSI high. */
                _SFR_IO8(port_mosi) |= (1 << pin_num_mosi);
            }
            else
            {
                /* Set the MOSI low. */
                _SFR_IO8(port_mosi) &= (uint8_t)(~(1 << pin_num_mosi));
            }

            /* Set SCLK high. */
            _SFR_IO8(port_sclk) |= (1 << pin_num_sclk);

            /* If MISO is high. */
            if ((_SFR_IO8(pin_miso) & (1 << pin_num_miso)) != 0)
            {
                /* Set this bit in RX register. */
                byte_rx |= 0x01;
            }

            /* Set SCLK low. */
            _SFR_IO8(port_sclk) &= (uint8_t)(~(1 << pin_num_sclk));

            /* A bit is now transfered */
            byte_tx = (byte_tx << 1);
        }

        /* Check if we are also reading. */
        if (need_read == TRUE)
        {
            /* Save the byte read from SPI. */
            *ptr = byte_rx;
        }

        /* Get next byte to send and update. */
        ptr++;
    }

    /* Return number of bytes written and read from SPI. */
    return (message->length);

} /* spi_bb_atmega644_message */

#endif /* CONFIG_SPI */
