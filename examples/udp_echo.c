/*
 * udp_echo.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <os.h>
#include <string.h>
#include <mem.h>
#include <fs.h>
#include <net.h>
#include <ppp.h>
#include <net_udp.h>

/* Function prototypes. */
void udp_echo_task(void *);

/* Buffer in which we will receive data. */
uint8_t result[1024];

/* UDP port structure. */
UDP_PORT udp_port;
SOCKET_ADDRESS socket_address;

void udp_echo_task(void *argv)
{
    int32_t received;

    /* Some compiler warnings. */
    UNUSED_PARAM(argv);

    while(1)
    {
        /* Initialize receive. */
        received = 0;

        /* Populate the socket structure. */
        socket_address.foreign_ip = 0xC0A80002;
        socket_address.foreign_port = 11001;
        socket_address.local_ip = 0xC0A80001;
        socket_address.local_port = 11000;

        /* Register this UDP port. */
        udp_register(&udp_port, "test", &socket_address);

        /* Run a UDP echo server. */
        while (received >= 0)
        {
            /* Receive some data from the UDP port. */
            received = fs_read(&udp_port, result, 1024);

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

    /* Initialize PPP stack. */
    ppp_init();

    /* Initialize networking stack. */
    net_init();

    /* Create a task for UDP echo demo. */
    udp_echo_task_cb = (TASK *)mem_static_alloc(sizeof(TASK) + 4096);
    task_create(udp_echo_task_cb, "ECHO", (uint8_t *)(udp_echo_task_cb + 1), 4096, &udp_echo_task, (void *)(NULL), 0);
    scheduler_task_add(udp_echo_task_cb, 5);

    /* Run scheduler. */
    os_run();

    return (0);

}
