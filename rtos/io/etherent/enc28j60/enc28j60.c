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

        /* Read the revision number. */
        OS_ASSERT(enc28j60_write_read_op(device, ENC28J60_OP_READ_CNTRL, ENC28J60_ADDR_EREVID, 0xFF, &value) != SUCCESS);

        /* We have initialized this device. */
        device->flags |= ENC28J60_FLAG_INIT;
    }

} /* enc28j60_process */

#endif /* CONFIG_ENC28J60 */
