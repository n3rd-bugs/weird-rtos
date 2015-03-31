/*
 * net.c
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

#ifdef CONFIG_NET
#include <net.h>

/*
 * net_init
 * This function will initialize the networking stack.
 */
void net_init()
{
    /* Initialize networking buffers file system. */
    net_buffer_init();

} /* net_buffer_init */

#endif /* CONFIG_NET */
