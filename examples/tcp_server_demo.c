/*
 * tcp_server_demo.c
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
#include <fs.h>
#include <net.h>
#include <net_tcp.h>
#include <sys_info.h>
#include <serial.h>

/* Demo configurations. */
#define DEMO_STACK_SIZE     512

/* Function prototypes. */
void tcp_server_task(void *);

/* TCP task stack. */
TASK tcp_task_cb;
uint8_t tcp_task_stack[DEMO_STACK_SIZE];

/* TCP port structure. */
TCP_PORT tcp_port;
SOCKET_ADDRESS socket_address;

void tcp_server_task(void *argv)
{
    /* Some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Clear the TCP port structure. */
    memset(&tcp_port, 0, sizeof(TCP_PORT));

    /* Populate the socket structure. */
    socket_address.foreign_ip = IPV4_ADDR_UNSPEC;
    socket_address.foreign_port = NET_PORT_UNSPEC;
    socket_address.local_ip = IPV4_ADDR_UNSPEC;
    socket_address.local_port = 11002;

    /* Register this TCP port. */
    tcp_register(&tcp_port, "test", &socket_address);

    /* Configure the TCP port to accept new connections. */
    tcp_listen(&tcp_port);

    /* Accept a TCP connection. */
    tcp_accept(&tcp_port, &tcp_port);
    printf("A TCP client has connected.\r\n");

    /* Send some data on the TCP port. */
    while (fs_write(&tcp_port, (uint8_t *)"HelloWorld", (int32_t)strlen("HelloWorld")) > 0) ;

    printf("Client closed the TCP connection.\r\n");

    /* Close this TCP port. */
    tcp_close(&tcp_port);
}

int main(void)
{
    memset(&tcp_task_cb, 0, sizeof(TASK));

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize file system. */
    fs_init();

    /* Initialize networking stack. */
    net_init();

    /* Initialize serial. */
    serial_init();

    /* Create a task for TCP demo. */
    task_create(&tcp_task_cb, P_STR("TCP"), tcp_task_stack, DEMO_STACK_SIZE, &tcp_server_task, (void *)(NULL), 0);
    scheduler_task_add(&tcp_task_cb, 5);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
