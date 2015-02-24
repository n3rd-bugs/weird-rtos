/*
 * usb_cdc_stm32f407.c
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

    /* Create a task for CDC demo. */
    cdc_demo_task_cb = (TASK *)mem_static_alloc(sizeof(TASK) + 4096);
    task_create(cdc_demo_task_cb, "STATS", (char *)(cdc_demo_task_cb + 1), 4096, &cdc_demo_task, (void *)(NULL));
    scheduler_task_add(cdc_demo_task_cb, TASK_APERIODIC, 5, 0);

    /* Run scheduler. */
    os_run();

    return (0);

}
