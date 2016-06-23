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
static void enc28j60_wdt(void *);
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
    ethernet_regsiter(&device->ethernet_device, &enc28j60_initialize, &enc28j60_transmit_packet, &enc28j60_interrupt, &enc28j60_wdt);

    /* Rather locking the global interrupts lock only the ethernet interrupts. */
    semaphore_set_irq_data(&device->ethernet_device.lock, device, (SEM_IRQ_LOCK *)&ENC28J60_DISABLE_INT, (SEM_IRQ_UNLOCK *)&ENC28J60_ENABLE_INT);

    /* Disable enc28j60 interrupts. */
    device->flags &= (uint8_t)~(ENC28J60_ENABLE_IRQ);

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
#else
    /* Set static IP address for this device. */
    ipv4_set_device_address(fd, 0xC0A80132);
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
    int32_t status;
    uint32_t max_retry = ENC28J60_CLKRDY_TIMEOUT / ENC28J60_CLKRDY_DELAY;

    /* Reset this device. */
    ENC28J60_RESET(device);

    /* Wait for clock signal. */
    do
    {
        /* Read the value of ESTAT. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, ENC28J60_ADDR_ESTAT, 0xFF, &value, 1);

#if ENC28J60_DEBUG
        printf("enc28j60_initialize ESTAT %d\r\n", value);
#endif

        /* If clock is still not ready. */
        if ((value & ENC28J60_ESTAT_CLKRDY) == 0)
        {
            sleep_ms(ENC28J60_CLKRDY_DELAY);
        }

        /* Decrement the number of retries. */
        max_retry --;

    } while ((status == SUCCESS) && (max_retry) && ((value & ENC28J60_ESTAT_CLKRDY) == 0));
    
    /* If we timed out while waiting for clock to enable. */
    if (!max_retry)
    {
        /* Device must not have been connected. */
        status = ENC28J60_DISCONNECTED;
    }

    if (status == SUCCESS)
    {
        /* Clear ECON1. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_ECON1, 0x00, NULL, 0);
    }

    if (status == SUCCESS)
    {
        /* Reset the memory block being used. */
        device->mem_block = 0;

        /* Read the revision number. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, ENC28J60_ADDR_EREVID, 0xFF, &value, 1);

#if ENC28J60_DEBUG
        printf("enc28j60_initialize REV_ID %d\r\n", value);
#endif
    }

    /* If we have a valid revision ID. */
    if ((status == SUCCESS) && (value == ENC28J60_REV_ID))
    {
        /* Enable address auto increment. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_ECON2, ENC28J60_ECON2_AUTOINC, NULL, 0);

        if (status == SUCCESS)
        {
            /* Enable unicast, broadcast and enable CRC validation. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_ERXFCON, (ENC28J60_ERXFCON_UCEN | ENC28J60_ERXFCON_BCEN | ENC28J60_ERXFCON_CRCEN), NULL, 0);
        }

        if (status == SUCCESS)
        {
            /* Enable MAC receive and flow control for RX and TX. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MACON1, (ENC28J60_MACON1_MARXEN | ENC28J60_MACON1_RXPAUS | ENC28J60_MACON1_TXPAUS), NULL, 0);
        }

        if (status == SUCCESS)
        {
            /* All short frames will be padded with 60-bytes and CRC will be
             * appended, enable frame length checking and enable full-duplex
             * mode. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MACON3, (ENC28J60_MACON3_PADCFG0 | ENC28J60_MACON3_TXCRCEN | ENC28J60_MACON3_FRMLNEN | ENC28J60_MACON3_FULDPX), NULL, 0);
        }

        if (status == SUCCESS)
        {
            /* Set MAIPGL to 0x12. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAIPGL, 0x12, NULL, 0);
        }

        if (status == SUCCESS)
        {
            /* Set MABBIPG to 0x15. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MABBIPG, 0x15, NULL, 0);
        }

        if (status == SUCCESS)
        {
            /* Set MAMXFLL/MAMXFLH to configured MTU. */
            status = enc28j60_write_word(device, ENC28J60_ADDR_MAMXFLL, ((net_device_get_mtu(fd) + ETH_HRD_SIZE) & 0xFFFF));
        }

        if (status == SUCCESS)
        {
            /* Generate a random MAC address and assign it to the device. */
            enc28j60_set_mac_address(device, ethernet_random_mac(&device->ethernet_device));

            /* Enable full-duplex mode on PHY. */
            status = enc28j60_write_phy(device, ENC28J60_ADDR_PHCON1, ENC28J60_PHCON1_PDPXMD);
        }

        if (status == SUCCESS)
        {
            /* Clear the PHCON2 register. */
            status = enc28j60_write_phy(device, ENC28J60_ADDR_PHCON2, 0);
        }

        if (status == SUCCESS)
        {
            /* Enable PHY interrupts with, link status change interrupt. */
            status = enc28j60_write_phy(device, ENC28J60_ADDR_PHIE, (ENC28J60_PHIE_PGEIE | ENC28J60_PHIE_PLNKIE));
        }

        if (status == SUCCESS)
        {
            /* Clear all interrupt requests. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_EIR, (ENC28J60_EIR_DMAIF | ENC28J60_EIR_LINKIF | ENC28J60_EIR_TXIF | ENC28J60_EIR_TXERIF | ENC28J60_EIR_RXERIF | ENC28J60_EIR_PKTIF), NULL, 0);
        }

        if (status == SUCCESS)
        {
            /* Enable global interrupts, with receive packet pending,
             * link status change, transmit enable and RX/TX error interrupts. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_EIE, (ENC28J60_EIE_INTIE | ENC28J60_EIE_PKTIE | ENC28J60_EIE_LINKIE | ENC28J60_EIE_TXIE | ENC28J60_EIE_TXERIE | ENC28J60_EIE_RXERIE), NULL, 0);
        }

        if (status == SUCCESS)
        {
            /* Enable PHY global interrupts with link change interrupt. */
            status = enc28j60_write_phy(device, ENC28J60_ADDR_PHIE, (ENC28J60_PHIE_PGEIE | ENC28J60_PHIE_PLNKIE));
        }

        if (status == SUCCESS)
        {
            /* Enable enc28j60 interrupts. */
            device->flags |= ENC28J60_ENABLE_IRQ;
        }
    }

} /* enc28j60_initialize */

/*
 * enc28j60_wdt
 * @data: ENC28J60 device instance for which an watch dog interrupt was
 *  triggered.
 * This function will process a target failure condition.
 */
static void enc28j60_wdt(void *data)
{
    ENC28J60 *device = (ENC28J60 *)data;

#if ENC28J60_DEBUG
    printf("enc28j60_wdt: forcefully unblocking TX.\r\n");
#endif

    /* Un-block TX as last frame was not sent. */
    device->flags &= (uint8_t)(~(ENC28J60_IN_TX));

    /* Process any blocked frames still to be sent. */
    device->ethernet_device.flags |= ETH_FLAG_TX;

} /* enc28j60_wdt */

/*
 * enc28j60_interrupt
 * @data: ENC28J60 device instance for which an interrupt is needed to be
 *  handled.
 * This function will process an interrupt for enc28j60 device.
 */
static void enc28j60_interrupt(void *data)
{
    ENC28J60 *device = (ENC28J60 *)data;
    int32_t status;
    FD fd = (FD)data;
    uint8_t value;

    /* Disable interrupts. */
    status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_EIE, ENC28J60_EIE_INTIE, NULL, 0);

    if (status == SUCCESS)
    {
        /* Get the interrupt status. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, ENC28J60_ADDR_EIR, 0xFF, &value, 1);

#if ENC28J60_DEBUG
    printf("enc28j60_interrupt: EIR 0x%d.\r\n", value);
#endif

        if (status == SUCCESS)
        {
            /* If link status has been changed. */
            if (value & ENC28J60_EIR_LINKIF)
            {
                /* Handle the link status change event. */
                enc28j60_link_changed(device);

                /* Clear the link status changed interrupt. */
                enc28j60_read_phy(device, ENC28J60_ADDR_PHIR, NULL);
            }

            /* ERRATA: The Receive Packet Pending Interrupt Flag (EIR.PKTIF) does not
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
                status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_EIR, ENC28J60_EIR_RXERIF, NULL, 0);
            }

            /* In case of TX complete or TX error unblock the TX. */
            if ((value & ENC28J60_EIR_TXIF) || (value & ENC28J60_EIR_TXERIF))
            {
                /* Disable watch dog interrupt. */
                ethernet_wdt_disable(&device->ethernet_device);

                /* Stop the TX. */
                status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_TXRTS, NULL, 0);

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
                status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_EIR, ENC28J60_EIR_TXIF, NULL, 0);
            }

            /* If a transmit error was detected. */
            if (value & ENC28J60_EIR_TXERIF)
            {
                /* Initialize TX FIFO. */
                status = enc28j60_tx_fifo_init(device);

                if (status == SUCCESS)
                {
                    /* Retransmit the old buffer. */
                    status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_TXRTS, NULL, 0);
                }

                if (status == SUCCESS)
                {
                    /* Clear the TX error interrupt. */
                    status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_EIR, ENC28J60_EIR_TXERIF, NULL, 0);
                }
            }
        }

        /* Enable enc28j60 interrupts. */
        device->flags |= ENC28J60_ENABLE_IRQ;

        /* Enable interrupts. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_EIE, ENC28J60_EIE_INTIE, NULL, 0);
    }

    /* If interrupt was not processed successfully. */
    if (status != SUCCESS)
    {
        /* Disable enc28j60 interrupts. */
        device->flags &= (uint8_t)~(ENC28J60_ENABLE_IRQ);

        /* Keep executing this interrupt until we have successfully processed
         * the interrupt. */
        /* This will help recover from a SPI error. */
        ethernet_interrupt(&device->ethernet_device);
    }

} /* enc28j60_interrupt */

/*
 * enc28j60_handle_rx_error
 * @device: ENC28J60 device instance for which RX error is needed to be handled.
 * This function will handle RX error on a enc28j60 device.
 */
static void enc28j60_handle_rx_error(ENC28J60 *device)
{
    int32_t status;

    /* Disable receive logic. */
    status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_CLR, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_RXEN, NULL, 0);

    if (status == SUCCESS)
    {
        /* Reinitialize RX FIFO. */
        status = enc28j60_rx_fifo_init(device);
    }

    if (status == SUCCESS)
    {
        /* Enable receive logic. */
        status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_RXEN, NULL, 0);
    }

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
    int32_t status;

    /* Update MAC address in the hardware. */
    status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR0, mac[5], NULL, 0);

    if (status == SUCCESS)
    {
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR1, mac[4], NULL, 0);
    }

    if (status == SUCCESS)
    {
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR2, mac[3], NULL, 0);
    }

    if (status == SUCCESS)
    {
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR3, mac[2], NULL, 0);
    }

    if (status == SUCCESS)
    {
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR4, mac[1], NULL, 0);
    }

    if (status == SUCCESS)
    {
        status = enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAADR5, mac[0], NULL, 0);
    }

} /* enc28j60_set_mac_address */

/*
 * enc28j60_link_changed
 * @device: ENC28J60 device instance for which link status has been changed.
 * This function will be called whenever a link status change is detected.
 */
static void enc28j60_link_changed(ENC28J60 *device)
{
    FD fd = (FD)&device->ethernet_device;
    int32_t status;
    uint16_t phy_register;

    /* Read the PHY status register. */
    status = enc28j60_read_phy(device, ENC28J60_ADDR_PHSTAT2, &phy_register);

    /* If we are now in connected state. */
    if ((status == SUCCESS) && (phy_register & ENC28J60_PHSTAT2_LSTAT))
    {
        /* Initialize RX FIFO. */
        status = enc28j60_rx_fifo_init(device);

        if (status == SUCCESS)
        {
            /* Enable receive logic. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_RXEN, NULL, 0);
        }

        if (status == SUCCESS)
        {
            /* Initialize TX FIFO. */
            status = enc28j60_tx_fifo_init(device);
        }

        if (status == SUCCESS)
        {
            /* Set link-up for this device. */
            net_device_link_up(fd);

            /* Process any frames needed to be sent. */
            device->ethernet_device.flags |= ETH_FLAG_TX;
        }
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
    int32_t status;
    uint16_t next_ptr, packet_status, packet_length;
    uint8_t num_packets, receive_header[ENC28J60_RX_HEAD_SIZE];

    /* Read number of packets available to read. */
    status = enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, ENC28J60_ADDR_EPKTCNT, 0xFF, &num_packets, 1);

    /* Process all the received packets. */
    while ((status == SUCCESS) && (num_packets --))
    {
        /* Read the receive buffer header. */
        status = enc28j60_read_buffer(device, device->rx_ptr, receive_header, ENC28J60_RX_HEAD_SIZE);

        if (status == SUCCESS)
        {
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
                            status = enc28j60_read_buffer(device, (uint16_t)(ENC28J60_RX_START_PTR(device->rx_ptr)), one_buffer->buffer, (int32_t)one_buffer->length);

                            if (status == SUCCESS)
                            {
                                /* Update the remaining number of bytes in the packet. */
                                packet_length = (uint16_t)(packet_length - one_buffer->length);
                                device->rx_ptr = (uint16_t)(device->rx_ptr + one_buffer->length);

                                /* Append this new buffer to the buffer chain. */
                                fs_buffer_add_one(buffer, one_buffer, 0);
                            }

                            else
                            {
                                /* Break and stop processing this packet. */
                                break;
                            }
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
        }

        if (status == SUCCESS)
        {
            /* Set new RX data pointer at ERXRDPTL/ERXRDPTH. */
            status = enc28j60_write_word(device, ENC28J60_ADDR_ERXRDPTL, (uint16_t)(ENC28J60_RX_PTR(next_ptr)));
        }

        if (status == SUCCESS)
        {
            /* Set the next packet pointer. */
            device->rx_ptr = next_ptr;

            /* Decrement the number of packets. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON2, ENC28J60_ECON2_PKTDEC, NULL, 0);
        }
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
        while ((status == SUCCESS) && (one != NULL))
        {
            /* Write this packet fragment. */
            status = enc28j60_write_buffer(device, tx_ptr, one->buffer, (int32_t)one->length);

            if (status == SUCCESS)
            {
                /* Increment the TX pointer. */
                tx_ptr = (uint16_t)(tx_ptr + one->length);

                /* Pick the next one buffer. */
                one = one->next;
            }
        }

        if (status == SUCCESS)
        {
            /* Set TX buffer end address at ETXNDL/ETXNDH. */
            status = enc28j60_write_word(device, ENC28J60_ADDR_ETXNDL, (uint16_t)(tx_ptr - 1));
        }

        if (status == SUCCESS)
        {
            /* Start the TX. */
            status = enc28j60_write_read_op(device, ENC28J60_OP_BIT_SET, ENC28J60_ADDR_ECON1, ENC28J60_ECON1_TXRTS, NULL, 0);
        }

        if (status == SUCCESS)
        {
            /* We are now transmitting a frame. */
            device->flags |= ENC28J60_IN_TX;

            /* Enable watch dog interrupt. */
            ethernet_wdt_enable(&device->ethernet_device, ENC28J60_WDT_TIMEOUT);
        }
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
