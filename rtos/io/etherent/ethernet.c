/*
 * spi.c
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
#include <ethernet.h>

/*
 * ethernet_init
 * This function will initialize ethernet devices.
 */
void ethernet_init()
{
    /* Do target initialization for ethernet. */
    ETHERENET_TGT_INIT();

} /* ethernet_init */

#endif /* CONFIG_ETHERNET */
