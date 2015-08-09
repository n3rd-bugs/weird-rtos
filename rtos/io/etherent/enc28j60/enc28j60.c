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
static void enc28j60_handle_rx_error(ENC28J60 *);
static int32_t enc28j60_tx_fifo_init(ENC28J60 *);
static int32_t enc28j60_rx_fifo_init(ENC28J60 *);
static void enc28j60_set_mac_address(ENC28J60 *, uint8_t *);
static void enc28j60_link_changed(ENC28J60 *);
static void enc28j60_receive_packet(ENC28J60 *);
static int32_t enc28j60_transmit_packet(void *, FS_BUFFER *);

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
    device->spi.baudrate = 21000000;
    device->spi.cfg_flags = (SPI_CFG_MASTER | SPI_CFG_CLK_FIRST_DATA);

    /* Do SPI initialization. */
    spi_init(&device->spi);

    /* Set the buffer data structure for this file descriptor. */
    fs_buffer_dataset(&device->ethernet_device, &device->fs_buffer_data, ENC28J60_NUM_BUFFERS);
    device->fs_buffer_data.threshold_buffers = ENC28J60_NUM_THR_BUFFER;

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
    ethernet_regsiter(&device->ethernet_device, &enc28j60_initialize, &enc28j60_transmit_packet, &enc28j60_interrupt);

#ifdef NET_ARP
    /* Set ARP data for this ethernet device. */
    arp_set_data(fd, device->arp_entries, ENC28J60_NUM_ARP);
#endif

#ifdef IPV4_ENABLE_FRAG
    /* Initialize fragment data for this device. */
    ipv4_fragment_set_data(fd, device->ipv4_fragments, ENC28J60_NUM_IPV4_FRAGS);
#endif

#ifdef DHCP_CLIENT
    /* Initialize the DHCP client data for this device. */
    net_dhcp_client_initialize_device(net_device_get_fd(fd), &device->dhcp_client);
#endif

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
        OS_ASSERT(enc28j60_write_word(device, ENC28J60_ADDR_MAMXFLL, ((net_device_get_mtu(fd) + ETH_HRD_SIZE) & 0xFFFF)) != SUCCESS);

        /* Generate a random MAC address and assign it to the device. */
        enc28j60_set_mac_address(device, ethernet_random_mac(&device->ethernet_device));

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

        /* Enable PHY global interrupts with link change interrupt. */
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
    FD fd = (FD)data;
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

    /* ERRACTA: The Receive Packet Pending Interrupt Flag (EIR.PKTIF) does not
     * reliably/accurately report the status of pending packets. */
    /* Solution: In the Interrupt Service Routine, if it is unknown if a packet
     * is pending and the source of the interrupt is unknown, switch to Bank 1
     * and check the value in EPKTCNT. */
    /* If a packet was received or an RX error was detected or source of
     * interrupt is unknown. */
    if ((value & ENC28J60_EIR_PKTIF) || (value & ENC28J60_EIR_RXERIF) || (value == 0x00))
    {
        /* Receive packets from the hardware. */
        enc28j60_receive_packet(device);
    }

    /* If an RX error was detected. */
    if (value & ENC28J60_EIR_RXERIF)
    {
        /* Handle RX error. */
        enc28j60_handle_rx_error(device);

        /* Clear the RX error interrupt. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_EIR, ENC28J60_EIR_RXERIF, NULL, 0) != SUCCESS);
    }

    /* In case of TX complete or TX error unblock the TX. */
    if ((value & ENC28J60_EIR_TXIF) || (value & ENC28J60_EIR_TXERIF))
    {
        /* Stop the TX. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_TXRTS, NULL, 0) != SUCCESS);

        /* A packet was successfully transmitted, un-block the TX. */
        device->ethernet_device.flags |= ETH_FLAG_TX;
        device->flags &= (uint8_t)(~(ENC28J60_IN_TX));

        /* Set event that will unblock any tasks waiting for it. */
        fd_data_available(fd);
    }

    /* If a packet was successfully transmitted. */
    if (value & ENC28J60_EIR_TXIF)
    {
        /* Clear the TX complete interrupt. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_EIR, ENC28J60_EIR_TXIF, NULL, 0) != SUCCESS);
    }

    /* If a transmit error was detected. */
    if (value & ENC28J60_EIR_TXERIF)
    {
        /* Initialize TX FIFO. */
        OS_ASSERT(enc28j60_tx_fifo_init(device) != SUCCESS);

        /* Retransmit the old buffer. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_TXRTS, NULL, 0) != SUCCESS);

        /* Clear the TX error interrupt. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_EIR, ENC28J60_EIR_TXERIF, NULL, 0) != SUCCESS);
    }

    /* Enable enc28j60 interrupts. */
    ENC28J60_ENABLE_INT(device);

    /* Enable interrupts. */
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_EIE, ENC28J60_EIE_INTIE, NULL, 0) != SUCCESS);

} /* enc28j60_interrupt */

/*
 * enc28j60_handle_rx_error
 * @device: ENC28J60 device instance for which RX error is needed to be handled.
 * This function will handle RX error on a enc28j60 device.
 */
static void enc28j60_handle_rx_error(ENC28J60 *device)
{
    /* Disable receive logic. */
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_RXEN, NULL, 0) != SUCCESS);

    /* Reinitialize RX FIFO. */
    OS_ASSERT(enc28j60_rx_fifo_init(device) != SUCCESS);

    /* Enable receive logic. */
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_RXEN, NULL, 0) != SUCCESS);

} /* enc28j60_handle_rx_error */

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

    /* Set TX reset bit. */
    status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_TXRST, NULL, 0);

    if (status == SUCCESS)
    {
        /* Clear TX reset bit. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_TXRST, NULL, 0);
    }

    if (status == SUCCESS)
    {
        /* Set TX buffer start address at ETXSTL/ETXSTH. */
        status = enc28j60_write_word(device, ENC28J60_ADDR_ETXSTL, ENC28J60_TX_START);
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

    /* Set RX reset bit. */
    status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_RXRST, NULL, 0);

    if (status == SUCCESS)
    {
        /* Clear RX reset bit. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_RXRST, NULL, 0);
    }

    if (status == SUCCESS)
    {
        /* Set RX buffer start address at ERXSTL/ERXSTH. */
        status = enc28j60_write_word(device, ENC28J60_ADDR_ERXSTL, ENC28J60_RX_START);
    }

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
 * enc28j60_set_mac
 * @device: ENC28J60 device instance for which MAC address is needed to be set.
 * @mac: MAC address needed to be set.
 * This function will update the MAC address for a ENC28J60 device.
 */
static void enc28j60_set_mac_address(ENC28J60 *device, uint8_t *mac)
{
    /* Update MAC address in the hardware. */
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR0, mac[5], NULL, 0) != SUCCESS);
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR1, mac[4], NULL, 0) != SUCCESS);
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR2, mac[3], NULL, 0) != SUCCESS);
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR3, mac[2], NULL, 0) != SUCCESS);
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR4, mac[1], NULL, 0) != SUCCESS);
    OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR5, mac[0], NULL, 0) != SUCCESS);

} /* enc28j60_set_mac_address */

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

        /* Initialize TX FIFO. */
        OS_ASSERT(enc28j60_tx_fifo_init(device) != SUCCESS);

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

        /* If we don't have an anticipated RX pointer. */
        if ((next_ptr & 0x01) || (next_ptr > ENC28J60_RX_END))
        {
            /* Handle RX error. */
            enc28j60_handle_rx_error(device);

            /* Break out of this loop. */
            break;
        }

        /* Save the received packet length. */
        packet_length = (uint16_t)((receive_header[3] << 8) | receive_header[2]);

        /* Save the packet status. */
        packet_status = (uint16_t)((receive_header[5] << 8) | receive_header[4]);

        /* If packet was successfully received. */
        if ((packet_status & ENC28J60_RX_RXOK) && (packet_length > ENC28J60_RX_CRC_LEN))
        {
            /* Pull a buffer list from the file descriptor. */
            buffer = fs_buffer_get(fd, FS_BUFFER_LIST, 0);

            /* If we do have a receive buffer. */
            if (buffer != NULL)
            {
                /* Don't copy the trailing CRC. */
                packet_length = (uint16_t)(packet_length - ENC28J60_RX_CRC_LEN);

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
                if ((packet_length != 0) || (buffer->total_length < ETH_HRD_SIZE) || (ethernet_buffer_receive(buffer) != NET_BUFFER_CONSUMED))
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

/*
 * enc28j60_transmit_packet
 * @data: ENC28J60 device instance on which a packet is needed to be
 *  transmitted.
 * @buffer: Buffer needed to be transmitted.
 * @return: A success status will be returned if packet was successfully queued
 *  for transmission, ETH_TX_BLOCKED will be returned to tell that TX is blocked
 *  and we don't have space to send this packet.
 * This function will send an ethernet frame on wire for a given enc28j60
 * device.
 */
static int32_t enc28j60_transmit_packet(void *data, FS_BUFFER *buffer)
{
    FS_BUFFER_ONE *one = buffer->list.head;
    ENC28J60 *device = (ENC28J60 *)data;
    int32_t status = SUCCESS;
    uint16_t tx_ptr = ENC28J60_TX_START;
    uint8_t value;

    /* If we are currently not doing any transmission. */
    if ((device->flags & ENC28J60_IN_TX) == 0)
    {
        /* Write the packet control register. */
        value = 0x00;
        enc28j60_write_buffer(device, tx_ptr++, &value, 1);

        /* Write the given buffer. */
        while (one != NULL)
        {
            /* Write this packet fragment. */
            OS_ASSERT(enc28j60_write_buffer(device, tx_ptr, one->buffer, (int32_t)one->length) != SUCCESS);

            /* Increment the TX pointer. */
            tx_ptr = (uint16_t)(tx_ptr + one->length);

            /* Pick the next one buffer. */
            one = one->next;
        }

        /* Set TX buffer end address at ETXNDL/ETXNDH. */
        OS_ASSERT(enc28j60_write_word(device, ENC28J60_ADDR_ETXNDL, (uint16_t)(tx_ptr - 1)) != SUCCESS);

        /* Start the TX. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_TXRTS, NULL, 0) != SUCCESS);

        /* We are now transmitting a frame. */
        device->flags |= ENC28J60_IN_TX;
    }
    else
    {
        /* TX is currently blocked. */
        status = ETH_TX_BLOCKED;
    }

    /* Return status to the caller. */
    return (status);

} /* enc28j60_transmit_packet */

#endif /* ETHERNET_ENC28J60 */
