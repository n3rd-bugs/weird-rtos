/*
 * enc28j60_demo_avr.c
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
#include <string.h>
#include <fs.h>
#include <net.h>
#include <net_udp.h>
#include <sys_info.h>

/* Demo configurations. */
#define DEMO_STACK_SIZE     384

/* Function prototypes. */
void udp_echo_task(void *);

/* UDD echo task stack. */
uint8_t udp_echo_task_stack[DEMO_STACK_SIZE];

/* UDP port structure. */
UDP_PORT udp_port;
SOCKET_ADDRESS socket_address;

void udp_echo_process(void *data)
{
    int32_t     received;
    FS_BUFFER   *rx_buffer;

    /* Receive some data from the UDP port. */
    received = fs_read(&udp_port, (uint8_t *)&rx_buffer, sizeof(FS_BUFFER));

    /* If some data was received. */
    if (received > 0)
    {
        /* Update the socket address. */
        udp_port.socket_address = udp_port.last_datagram_address;

        /* Send received data back on the UDP port. */
        received = fs_write(&udp_port, (uint8_t *)rx_buffer, sizeof(FS_BUFFER));
    }
}

void udp_echo_task(void *argv)
{
    CONDITION   *udp_condition;
    SUSPEND     udp_suspend;
    FS_PARAM    udp_fs_param;

    memset(&udp_port, 0, sizeof(UDP_PORT));

    /* Use buffered mode for this UDP port. */
    udp_port.console.fs.flags = FS_BUFFERED;

    /* Some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Populate the socket structure. */
    socket_address.foreign_ip = IPV4_ADDR_UNSPEC;
    socket_address.foreign_port = 11001;
    socket_address.local_ip = IPV4_ADDR_UNSPEC;
    socket_address.local_port = 11000;

    /* Register this UDP port. */
    udp_register((FD)&udp_port, "test", &socket_address);

    /* Get read condition for this UDP port. */
    fs_condition_get((FD)&udp_port, &udp_condition, &udp_suspend, &udp_fs_param, FS_BLOCK_READ);

    /* Add a networking condition for this UDP port. */
    net_condition_add(udp_condition, &udp_suspend, &udp_echo_process, (FD)&udp_port);

    while(1)
    {
        /* Print system statistics. */
        util_print_sys_info();

        /* Sleep for a second. */
        sleep_ms(1000);
    }
}

int main(void)
{
    TASK udp_echo_task_cb;

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize file system. */
    fs_init();

    /* Initialize networking stack. */
    net_init();

    /* Create a task for UDP echo demo. */
    task_create(&udp_echo_task_cb, "ECHO", udp_echo_task_stack, DEMO_STACK_SIZE, &udp_echo_task, (void *)(NULL), 0);
    scheduler_task_add(&udp_echo_task_cb, 5);

    /* Run scheduler. */
    os_run();

    return (0);

}
