/*
 * tftps_demo.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <kernel.h>
#include <string.h>
#include <fs.h>
#include <net.h>
#include <math.h>
#include <fs.h>
#include <sys_info.h>
#include <tftps.h>
#include <serial.h>

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

#ifdef IO_SERIAL
    /* Initialize serial. */
    serial_init();
#endif

    /* Populate the socket structure. */
    socket_address.foreign_ip = IPV4_ADDR_UNSPEC;
    socket_address.foreign_port = NET_PORT_UNSPEC;
    socket_address.local_ip = IPV4_ADDR_UNSPEC;
    socket_address.local_port = 69;

    /* Initialize TFTP server. */
    tftp_server_init(&tftp_server, &socket_address, "tftp");

    /* Run scheduler. */
    kernel_run();

    return (0);

}
