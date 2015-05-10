/*
 * ethernet_stm32f407.c
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

#ifdef CONFIG_ETHERNET
#include <ethernet_stm32f407.h>
#ifdef CONFIG_ENC28J60
#include <enc28j60.h>
ENC28J60 enc28j60;
#endif
#include <string.h>

/*
 * ethernet_stm32f407_init
 * This function will initialize ethernet devices for stm32f407 platform.
 */
void ethernet_stm32f407_init()
{
#ifdef CONFIG_ENC28J60
    /* Clear the enc28j60 device structure. */
    memset(&enc28j60, 0, sizeof(ENC28J60));

    /* Initialize SPI device data. */
    enc28j60.spi.data.device_num = 1;

    /* Do enc28j60 initialization. */
    enc28j60_init(&enc28j60);
#endif

} /* ethernet_stm32f407_init */

#endif /* CONFIG_ETHERNET */
