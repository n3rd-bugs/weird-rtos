/*
 * enc28j60_demo.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>
#include <string.h>
#include <mem.h>
#include <fs.h>
#include <net.h>
#include <net_udp.h>
#include <serial.h>

/* Function prototypes. */
void udp_echo_task(void *);

/* Buffer in which we will receive data. */
uint8_t result[1500];

/* UDP port structure. */
UDP_PORT udp_port;
SOCKET_ADDRESS socket_address;

void udp_echo_task(void *argv)
{
    int32_t received;

    memset(&udp_port, 0, sizeof(UDP_PORT));

    /* Some compiler warnings. */
    UNUSED_PARAM(argv);

    while(1)
    {
        /* Initialize receive. */
        received = 0;

        /* Populate the socket structure. */
        socket_address.foreign_ip = 0xC0A80001;
        socket_address.foreign_port = 11001;
        socket_address.local_ip = 0xC0A80003;
        socket_address.local_port = 11000;

        /* Register this UDP port. */
        udp_register(&udp_port, "test", &socket_address);

        /* Run a UDP echo server. */
        while (received >= 0)
        {
            /* Receive some data from the UDP port. */
            received = fs_read(&udp_port, result, 1500);

            /* If some data was received. */
            if (received > 0)
            {
                /* Update the socket address. */
                udp_port.socket_address = udp_port.last_datagram_address;

                /* Send this data back on the UDP port. */
                received = fs_write(&udp_port, result, received);
            }
        }

        /* Unregister UDP port. */
        udp_unregister(&udp_port);
    }
}

int main(void)
{
    TASK *udp_echo_task_cb;

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize memory. */
    mem_init();

    /* Initialize file system. */
    fs_init();

    /* Initialize networking stack. */
    net_init();

#ifdef IO_SERIAL
    /* Initialize serial. */
    serial_init();
#endif

    /* Create a task for UDP echo demo. */
    udp_echo_task_cb = (TASK *)mem_static_alloc(sizeof(TASK) + 4096);
    task_create(udp_echo_task_cb, P_STR("ECHO"), (uint8_t *)(udp_echo_task_cb + 1), 4096, &udp_echo_task, (void *)(NULL), 0);
    scheduler_task_add(udp_echo_task_cb, 5);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
