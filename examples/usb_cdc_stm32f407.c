/*
 * usb_cdc_stm32f407.c
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
#include <serial.h>

void cdc_demo_task(void *);

void cdc_demo_task(void *argv)
{
    UNUSED_PARAM(argv);

    while(1)
    {
        printf("1234567890123456789012345678901234567890\r\n");
    }
}

int main(void)
{
    TASK *cdc_demo_task_cb;

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize memory. */
    mem_init();

    /* Initialize file system. */
    fs_init();

    /* Initialize USB stack. */
    usb_init();

    /* Initialize serial. */
    serial_init();

    /* Create a task for CDC demo. */
    cdc_demo_task_cb = (TASK *)mem_static_alloc(sizeof(TASK) + 4096);
    task_create(cdc_demo_task_cb, P_STR("CDCDEMO"), (uint8_t *)(cdc_demo_task_cb + 1), 4096, &cdc_demo_task, (void *)(NULL));
    scheduler_task_add(cdc_demo_task_cb, 5);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
