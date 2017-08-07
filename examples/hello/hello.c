/*
 * hello.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#include <kernel.h>
#include <string.h>
#include <fs.h>
#include <serial.h>
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
