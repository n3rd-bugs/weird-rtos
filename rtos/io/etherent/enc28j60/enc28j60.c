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

#ifdef CONFIG_ENC28J60
#include <enc28j60.h>
#include <enc28j60_spi.h>
#include <net_condition.h>

/* Internal function prototypes. */
static uint8_t enc28j60_do_suspend(void *, void *);
static void enc28j60_process(void *);

/*
 * enc28j60_init
 * @device: ENC28J60 device instance needed to be initialized.
 * This function will initialize an enc28j60 ethernet controller.
 */
void enc28j60_init(ENC28J60 *device)
{
    /* Initialize SPI parameters. */
    device->spi.baudrate = 20000000;
    device->spi.cfg_flags = (SPI_CFG_MASTER | SPI_CFG_CLK_FIRST_DATA);

    /* Do SPI initialization. */
    spi_init(&device->spi);

    /* Initialize device suspend condition. */
    device->condition.data = device;
    device->suspend.do_suspend = &enc28j60_do_suspend;
    device->suspend.param = NULL;   /* For now unused. */
    device->suspend.timeout = MAX_WAIT;

    /* Set the device MTU. */
    device->mtu = ENC28J60_MTU;

    /* Add networking condition to further process this ethernet device. */
    net_condition_add(&device->condition, &device->suspend, &enc28j60_process, device);

} /* enc28j60_init */

/*
 * enc28j60_do_suspend
 * @data: ENC28J60 device instance.
 * @suspend_data: Suspend data for this enc28j60 device.
 * @return: Will return true if we can suspend on this device otherwise false
 *  will be returned.
 * This function will check if we need to suspend on this enc28k60 device.
 */
static uint8_t enc28j60_do_suspend(void *data, void *suspend_data)
{
    ENC28J60 *device = (ENC28J60 *)data;
    uint8_t do_suspend = TRUE;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(suspend_data);

    /* If this device is not yet initialized. */
    if ((device->flags & ENC28J60_FLAG_INIT) == 0)
    {
        /* Don't suspend and initialize this device. */
        do_suspend = FALSE;
    }

    /* Return if we need to suspend on this enc28k60 device. */
    return (do_suspend);

} /* enc28j60_do_suspend */

/*
 * enc28j60_process
 * @data: ENC28J60 device instance for which this function was called.
 * This function will process condition for ENC28J60 device.
 */
static void enc28j60_process(void *data)
{
    ENC28J60 *device = (ENC28J60 *)data;
    uint8_t value;

    /* If this device is not yet initialized. */
    if ((device->flags & ENC28J60_FLAG_INIT) == 0)
    {
        /* Reset this device. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_RESET, ENC28J60_ADDR_RESET, ENC28J60_VALUE_RESET, NULL) != SUCCESS);

        /* For now we need to sleep to wait for this device to initialize. */
        sleep_ms(50);

        /* Clear ECON1. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_ECON1, 0x00, NULL) != SUCCESS);

        /* Reset the memory block being used. */
        device->mem_block = 0;

        /* Read the revision number. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_READ_CTRL, ENC28J60_ADDR_EREVID, 0xFF, &value) != SUCCESS);

        /* If we have a valid revision ID. */
        if (value == ENC28J60_REV_ID)
        {
            /* Enable address auto increment. */
            OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_ECON2, ENC28J60_ECON2_AUTOINC, NULL) != SUCCESS);

            /* Enable unicast, broadcast and enable CRC validation. */
            OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_ERXFCON, (ENC28J60_ERXFCON_UCEN | ENC28J60_ERXFCON_BCEN | ENC28J60_ERXFCON_CRCEN), NULL) != SUCCESS);

            /* All short frames will be padded with 60-bytes and CRC will be
             * appended, enable frame length checking and enable full-duplex
             * mode. */
            OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MACON3, (ENC28J60_MACON3_PADCFG0 | ENC28J60_MACON3_TXCRCEN | ENC28J60_MACON3_FRMLNEN | ENC28J60_MACON3_FULDPX), NULL) != SUCCESS);

            /* Set MAIPGL to 0x12. */
            OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAIPGL, 0x12, NULL) != SUCCESS);

            /* Set MABBIPG to 0x15. */
            OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MABBIPG, 0x15, NULL) != SUCCESS);

            /* Set MAMXFLL/MAMXFLH to configured MTU. */
            OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAMXFLL, (device->mtu & 0xFF), NULL) != SUCCESS);
            OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_WRITE_CTRL, ENC28J60_ADDR_MAMXFLH, (uint8_t)(device->mtu >> 8), NULL) != SUCCESS);

            /* Enable full-duplex mode on PHY. */
            OS_ASSERT(enc28j60_write_phy(device, ENC28J60_ADDR_PHCON1, ENC28J60_PHCON1_PDPXMD) != SUCCESS);

            /* Clear the PHCON2 register. */
            OS_ASSERT(enc28j60_write_phy(device, ENC28J60_ADDR_PHCON2, 0) != SUCCESS);

            /* We have initialized this device. */
            device->flags |= ENC28J60_FLAG_INIT;
        }
        else
        {
            /* Non-supported or invalid revision ID was read. */

            /* Remove this device from the networking condition. */
            net_condition_remove(&device->condition);
        }
    }

} /* enc28j60_process */

#endif /* CONFIG_ENC28J60 */
