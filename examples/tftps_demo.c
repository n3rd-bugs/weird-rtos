/*
 * tftps_demo.c
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
#include <stdio.h>
#include <stdlib.h>
#include <os.h>
#include <string.h>
#include <fs.h>
#include <net.h>
#include <math.h>
#include <fs.h>
#include <sys_info.h>
#include <tftps.h>

/* TFTP server instance. */
TFTP_SERVER tftp_server;

/* Main entry function. */
int main(void)
{
    SOCKET_ADDRESS socket_address;

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize file system. */
    fs_init();

    /* Initialize networking stack. */
    net_init();

    /* Populate the socket structure. */
    socket_address.foreign_ip = IPV4_ADDR_UNSPEC;
    socket_address.foreign_port = NET_PORT_UNSPEC;
    socket_address.local_ip = IPV4_ADDR_UNSPEC;
    socket_address.local_port = 69;

    /* Initialize TFTP server. */
    tftp_server_init(&tftp_server, &socket_address, "tftp");

    /* Run scheduler. */
    os_run();

    return (0);

}
