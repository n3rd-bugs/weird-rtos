/*
 * enc28j60.c
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
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>
#include <enc28j60_spi.h>

/* Internal function prototypes. */
static void enc28j60_initialize(void *);
static void enc28j60_interrupt(void *);
static int32_t enc28j60_tx_fifo_init(ENC28J60 *);
static int32_t enc28j60_rx_fifo_init(ENC28J60 *);
static void enc28j60_link_changed(ENC28J60 *);
static void enc28j60_receive_packet(ENC28J60 *);

/*
 * enc28j60_init
 * @device: ENC28J60 device instance needed to be initialized.
 * This function will initialize an enc28j60 ethernet controller.
 */
void enc28j60_init(ENC28J60 *device)
{
    uint32_t i;
    FD fd = (FD)&device->ethernet_device;

    /* Initialize SPI parameters. */
    device->spi.baudrate = 20000000;
    device->spi.cfg_flags = (SPI_CFG_MASTER | SPI_CFG_CLK_FIRST_DATA);

    /* Do SPI initialization. */
    spi_init(&device->spi);

    /* Set the buffer data structure for this file descriptor. */
    fs_buffer_dataset(&device->ethernet_device, &device->fs_buffer_data, ENC28J60_NUM_BUFFERS);

    /* Add buffer for this console. */
    for (i = 0; i < ENC28J60_NUM_BUFFERS; i++)
    {
        /* Initialize a buffer. */
        fs_buffer_one_init(&device->fs_buffer[i], &device->buffer[ENC28J60_MAX_BUFFER_SIZE * i], ENC28J60_MAX_BUFFER_SIZE);

        /* Add this buffer to the free buffer list for this file descriptor. */
        fs_buffer_add(fd, &device->fs_buffer[i], FS_BUFFER_ONE_FREE, FS_BUFFER_ACTIVE);
    }

    /* Add buffer lists for this console. */
    for (i = 0; i < ENC28J60_NUM_BUFFER_LISTS; i++)
    {
        /* Initialize a buffer. */
        fs_buffer_init(&device->fs_buffer_list[i], fd);

        /* Add this buffer to the free buffer list for this file descriptor. */
        fs_buffer_add(fd, &device->fs_buffer_list[i], FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
    }

    /* Register this ethernet device. */
    ethernet_regsiter(&device->ethernet_device, &enc28j60_initialize, &enc28j60_interrupt);

} /* enc28j60_init */

/*
 * enc28j60_initialize
 * @data: ENC28J60 device instance needed to be initialized.
 * This function will initialize a enc28j60 device.
 */
static void enc28j60_initialize(void *data)
{
    ENC28J60 *device = (ENC28J60 *)data;
    uint8_t value;
    FD fd = (FD)&device->ethernet_device;

    /* Reset this device. */
    ENC28J60_RESET(device);

    /* Wait for clock signal. */
    do
    {
        /* Read the value of ESTAT. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, ENC28J60_ADDR_ESTAT, 0xFF, &value, 1) != SUCCESS);

    } while ((value & ENC28J60_ESTAT_CLKRDY) == 0);

    /* Clear ECON1. */
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_ECON1, 0x00, NULL, 0) != SUCCESS);

    /* Reset the memory block being used. */
    device->mem_block = 0;

    /* Read the revision number. */
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, ENC28J60_ADDR_EREVID, 0xFF, &value, 1) != SUCCESS);

    /* If we have a valid revision ID. */
    if (value == ENC28J60_REV_ID)
    {
        /* Enable address auto increment. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_ECON2, ENC28J60_ECON2_AUTOINC, NULL, 0) != SUCCESS);

        /* Initialize TX FIFO. */
        OS_ASSERT(enc28j60_tx_fifo_init(device) != SUCCESS);

        /* Enable unicast, broadcast and enable CRC validation. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_ERXFCON, (ENC28J60_ERXFCON_UCEN | ENC28J60_ERXFCON_BCEN | ENC28J60_ERXFCON_CRCEN), NULL, 0) != SUCCESS);

        /* Enable MAC receive and flow control for RX and TX. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MACON1, (ENC28J60_MACON1_MARXEN | ENC28J60_MACON1_RXPAUS | ENC28J60_MACON1_TXPAUS), NULL, 0) != SUCCESS);

        /* All short frames will be padded with 60-bytes and CRC will be
         * appended, enable frame length checking and enable full-duplex
         * mode. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MACON3, (ENC28J60_MACON3_PADCFG0 | ENC28J60_MACON3_TXCRCEN | ENC28J60_MACON3_FRMLNEN | ENC28J60_MACON3_FULDPX), NULL, 0) != SUCCESS);

        /* Set MAIPGL to 0x12. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAIPGL, 0x12, NULL, 0) != SUCCESS);

        /* Set MABBIPG to 0x15. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MABBIPG, 0x15, NULL, 0) != SUCCESS);

        /* Set MAMXFLL/MAMXFLH to configured MTU. */
        OS_ASSERT(enc28j60_write_word(device, ENC28J60_ADDR_MAMXFLL, (net_device_get_mtu(fd) & 0xFFFF)) != SUCCESS);

        /* Enable full-duplex mode on PHY. */
        OS_ASSERT(enc28j60_write_phy(device, ENC28J60_ADDR_PHCON1, ENC28J60_PHCON1_PDPXMD) != SUCCESS);

        /* Clear the PHCON2 register. */
        OS_ASSERT(enc28j60_write_phy(device, ENC28J60_ADDR_PHCON2, 0) != SUCCESS);

        /* Enable PHY interrupts with, link status change interrupt. */
        OS_ASSERT(enc28j60_write_phy(device, ENC28J60_ADDR_PHIE, (ENC28J60_PHIE_PGEIE | ENC28J60_PHIE_PLNKIE)) != SUCCESS);

        /* Clear all interrupt requests. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_EIR, (ENC28J60_EIR_DMAIF | ENC28J60_EIR_LINKIF | ENC28J60_EIR_TXIF | ENC28J60_EIR_TXERIF | ENC28J60_EIR_RXERIF | ENC28J60_EIR_PKTIF), NULL, 0) != SUCCESS);

        /* Enable global interrupts, with receive packet pending,
         * link status change, transmit enable and RX/TX error interrupts. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_EIE, (ENC28J60_EIE_INTIE | ENC28J60_EIE_PKTIE | ENC28J60_EIE_LINKIE | ENC28J60_EIE_TXIE | ENC28J60_EIE_TXERIE | ENC28J60_EIE_RXERIE), NULL, 0) != SUCCESS);

        /* Assign a random MAC address to this device. */
        OS_ASSERT(enc28j60_write_phy(device, ENC28J60_ADDR_PHIE, (ENC28J60_PHIE_PGEIE | ENC28J60_PHIE_PLNKIE)) != SUCCESS);

        /* Enable enc28j60 interrupts. */
        ENC28J60_ENABLE_INT(device);
    }

} /* enc28j60_initialize */

/*
 * enc28j60_interrupt
 * @data: ENC28J60 device instance for which an interrupt is needed to be
 *  handled.
 * This function will process an interrupt for enc28j60 device.
 */
static void enc28j60_interrupt(void *data)
{
    ENC28J60 *device = (ENC28J60 *)data;
    uint8_t value;

    /* Disable interrupts. */
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_EIE, ENC28J60_EIE_INTIE, NULL, 0) != SUCCESS);

    /* Get the interrupt status. */
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, ENC28J60_ADDR_EIR, 0xFF, &value, 1) != SUCCESS);

    /* If link status has been changed. */
    if (value & ENC28J60_EIR_LINKIF)
    {
        /* Handle the link status change event. */
        enc28j60_link_changed(device);

        /* Clear the link status changed interrupt. */
        enc28j60_read_phy(device, ENC28J60_ADDR_PHIR, NULL);
    }

    /* If a packet was received. */
    if (value & ENC28J60_EIR_PKTIF)
    {
        /* Receive a packet from the hardware. */
        enc28j60_receive_packet(device);
    }

    /* For now these are not handled. */
    OS_ASSERT((value & (ENC28J60_EIR_TXERIF | ENC28J60_EIR_RXERIF)));

    /* Enable enc28j60 interrupts. */
    ENC28J60_ENABLE_INT(device);

    /* Enable interrupts. */
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_EIE, ENC28J60_EIE_INTIE, NULL, 0) != SUCCESS);

} /* enc28j60_interrupt */

/*
 * enc28j60_tx_fifo_init
 * @device: ENC28J60 device instance for which TX FIFO is needed to be
 *  initialized.
 * @return: A success status will be returned TX FIFO was successfully
 *  initialized.
 * This function will initialize TX FIFO for given ENC28J60 device.
 */
static int32_t enc28j60_tx_fifo_init(ENC28J60 *device)
{
    int32_t status;

    /* Set TX buffer start address at ETXSTL/ETXSTH. */
    status = enc28j60_write_word(device, ENC28J60_ADDR_ETXSTL, ENC28J60_TX_START);

    if (status == SUCCESS)
    {
        /* Set TX buffer end address at ETXNDL/ETXNDH. */
        status = enc28j60_write_word(device, ENC28J60_ADDR_ETXNDL, ENC28J60_TX_END);
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_tx_fifo_init */

/*
 * enc28j60_rx_fifo_init
 * @device: ENC28J60 device instance for which RX FIFO is needed to be
 *  initialized.
 * @return: A success status will be returned RX FIFO was successfully
 *  initialized.
 * This function will initialize RX FIFO for given ENC28J60 device.
 */
static int32_t enc28j60_rx_fifo_init(ENC28J60 *device)
{
    int32_t status;

    /* Set RX buffer start address at ERXSTL/ERXSTH. */
    status = enc28j60_write_word(device, ENC28J60_ADDR_ERXSTL, ENC28J60_RX_START);

    if (status == SUCCESS)
    {
        /* Set RX buffer end address at ERXNDL/ERXNDH. */
        status = enc28j60_write_word(device, ENC28J60_ADDR_ERXNDL, ENC28J60_RX_END);
    }

    if (status == SUCCESS)
    {
        /* Set RX data pointer at ERXRDPTL/ERXRDPTH. */
        status = enc28j60_write_word(device, ENC28J60_ADDR_ERXRDPTL, ENC28J60_RX_PTR(ENC28J60_RX_START));
    }

    if (status == SUCCESS)
    {
        /* Save the receive pointer. */
        device->rx_ptr = ENC28J60_RX_START;
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_rx_fifo_init */

/*
 * enc28j60_link_changed
 * @device: ENC28J60 device instance for which link status has been changed.
 * This function will be called whenever a link status change is detected.
 */
static void enc28j60_link_changed(ENC28J60 *device)
{
    uint16_t phy_register;
    FD fd = (FD)&device->ethernet_device;

    /* Read the PHY status register. */
    OS_ASSERT(enc28j60_read_phy(device, ENC28J60_ADDR_PHSTAT2, &phy_register) != SUCCESS);

    /* If we are now in connected state. */
    if (phy_register & ENC28J60_PHSTAT2_LSTAT)
    {
        /* Initialize RX FIFO. */
        OS_ASSERT(enc28j60_rx_fifo_init(device) != SUCCESS);

        /* Enable receive logic. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_RXEN, NULL, 0) != SUCCESS);

        /* Set link-up for this device. */
        net_device_link_up(fd);
    }
    else
    {
        /* Set link-down for this device. */
        net_device_link_down(fd);
    }

} /* enc28j60_link_changed */

/*
 * enc28j60_receive_packet
 * @device: ENC28J60 device instance on which a packet was received.
 * This function will be called whenever a new packet is received.
 */
static void enc28j60_receive_packet(ENC28J60 *device)
{
    FS_BUFFER *buffer;
    FS_BUFFER_ONE *one_buffer;
    FD fd = (FD)&device->ethernet_device;
    uint16_t next_ptr, packet_status, packet_length;
    uint8_t num_packets, receive_header[ENC28J60_RX_HEAD_SIZE];

    /* Read number of packets available to read. */
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, ENC28J60_ADDR_EPKTCNT, 0xFF, &num_packets, 1) != SUCCESS);

    /* Process all the received packets. */
    while (num_packets --)
    {
        /* Read the receive buffer header. */
        OS_ASSERT(enc28j60_read_buffer(device, device->rx_ptr, receive_header, ENC28J60_RX_HEAD_SIZE) != SUCCESS);

        /* Save the next packet pointer. */
        next_ptr = (uint16_t)((receive_header[1] << 8) | receive_header[0]);

        /* Save the received packet length. */
        packet_length = (uint16_t)((receive_header[3] << 8) | receive_header[2]);

        /* Save the packet status. */
        packet_status = (uint16_t)((receive_header[5] << 8) | receive_header[4]);

        /* If packet was successfully received. */
        if (packet_status & ENC28J60_RX_RXOK)
        {
            /* Pull a buffer list from the file descriptor. */
            buffer = fs_buffer_get(fd, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

            /* If we do have a receive buffer. */
            if (buffer != NULL)
            {
                /* While we have some data to copy. */
                while (packet_length > 0)
                {
                    /* Pull a one buffer in which we will copy the data. */
                    one_buffer = fs_buffer_one_get(fd, FS_BUFFER_ONE_FREE, 0);

                    /* If we do have a buffer to copy data. */
                    if (one_buffer != NULL)
                    {
                        /* If we need to copy more data then we can actually
                         * receive in a buffer. */
                        if (packet_length > one_buffer->max_length)
                        {
                            /* Copy maximum number of bytes we can copy in the
                             * buffer. */
                            one_buffer->length = one_buffer->max_length;
                        }
                        else
                        {
                            /* Only copy remaining packet length. */
                            one_buffer->length = packet_length;
                        }

                        /* Read the received buffer. */
                        OS_ASSERT(enc28j60_read_buffer(device, (uint16_t)(ENC28J60_RX_START_PTR(device->rx_ptr)), one_buffer->buffer, (int32_t)one_buffer->length) != SUCCESS);

                        /* Update the remaining number of bytes in the packet. */
                        packet_length = (uint16_t)(packet_length - one_buffer->length);
                        device->rx_ptr = (uint16_t)(device->rx_ptr + one_buffer->length);

                        /* Append this new buffer to the buffer chain. */
                        fs_buffer_add_one(buffer, one_buffer, 0);
                    }
                    else
                    {
                        /* We don't have enough buffers to process this packet. */
                        break;
                    }
                }

                /* If we were not able to receive a complete packet due to
                 * unavailability of buffer, or ethernet stack did not consume
                 * this buffer. */
                if ((packet_length != 0) || (ethernet_buffer_receive(buffer) != NET_BUFFER_CONSUMED))
                {
                    /* Free the buffers that we allocated. */
                    fs_buffer_add(buffer->fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
                }
            }
        }

        /* Set new RX data pointer at ERXRDPTL/ERXRDPTH. */
        OS_ASSERT(enc28j60_write_word(device, ENC28J60_ADDR_ERXRDPTL, (uint16_t)(ENC28J60_RX_PTR(next_ptr))) != SUCCESS);

        /* Set the next packet pointer. */
        device->rx_ptr = next_ptr;

        /* Decrement the number of packets. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON2, ENC28J60_ECON2_PKTDEC, NULL, 0) != SUCCESS);
    }

} /* enc28j60_receive_packet */

#endif /* ETHERNET_ENC28J60 */
