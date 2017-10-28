/*
 * hello.c
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
#include <kernel.h>
#include <string.h>
#ifdef CONFIG_FS
#include <fs.h>
#endif /* CONFIG_FS */
#ifdef CONFIG_SERIAL
#include <serial.h>
#endif /* CONFIG_SERIAL */
#include <stdio.h>

/* Function prototypes. */
void hello_task(void *);

/* Hello task control block. */
/* TODO adjust this if serial is enabled. */
#define DEMO_STACK_SIZE     192
TASK hello_task_cb;
uint8_t hello_task_stack[DEMO_STACK_SIZE];

void hello_task(void *argv)
{
    /* Some compiler warnings. */
    UNUSED_PARAM(argv);

    while(1)
    {
#ifdef CONFIG_SERIAL
        printf("Hello World\r\n");
#endif /* CONFIG_SERIAL */

        /* Sleep for some time. */
        sleep_ms(500);
    }
}

int main(void)
{
    /* Initialize scheduler. */
    scheduler_init();

#ifdef CONFIG_SERIAL
#ifdef CONFIG_FS
    /* Initialize file system. */
    fs_init();
#endif /* CONFIG_FS */

    /* Initialize serial. */
    serial_init();
#endif /* CONFIG_SERIAL */

    /* Create a task for hello demo. */
    task_create(&hello_task_cb, P_STR("HELLO"), hello_task_stack, DEMO_STACK_SIZE, &hello_task, (void *)(NULL), 0);
    scheduler_task_add(&hello_task_cb, 5);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
