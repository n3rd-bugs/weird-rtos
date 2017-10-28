/*
 * ssd1306.c
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
#include <stdio.h>
#ifdef CONFIG_FS
#include <fs.h>
#endif /* CONFIG_FS */
#ifdef CONFIG_SERIAL
#include <serial.h>
#endif /* CONFIG_SERIAL */
#include <oled_ssd1306.h>

/* Function prototypes. */
void demo_task(void *);

/* Hello task control block. */
#define DEMO_STACK_SIZE     512
TASK demo_task_cb;
uint8_t demo_task_stack[DEMO_STACK_SIZE];

void demo_task(void *argv)
{
    /* Some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Initialize OLED. */
    oled_ssd1306_init();

    while(1)
    {
        /* Sleep for some time. */
        sleep_ms(500);
    }
}

int main(void)
{
    /* Initialize scheduler. */
    scheduler_init();

#ifdef CONFIG_FS
    /* Initialize file system. */
    fs_init();
#endif /* CONFIG_FS */

#ifdef CONFIG_SERIAL
    /* Initialize serial. */
    serial_init();
#endif /* CONFIG_SERIAL */

    /* Create a task for demo. */
    task_create(&demo_task_cb, P_STR("HELLO"), demo_task_stack, DEMO_STACK_SIZE, &demo_task, (void *)(NULL), 0);
    scheduler_task_add(&demo_task_cb, 5);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
