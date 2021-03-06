/*
 * fat_demo.c
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
#include <serial.h>

/* FAT FS demo task definitions. */
#define FAT_DEMO_STACK_SIZE     1024
uint8_t fat_demo_stack[FAT_DEMO_STACK_SIZE];
TASK    fat_demo_cb;
void fat_demo_entry(void *argv);

#define FAT_DEMO_FILE_SIZE      (1024UL * 1024UL * 1UL) /* 1 MB */
uint8_t test_buffer[64];

/*
 * fat_demo_entry
 * @argv: Task argument.
 * This is main entry function for FAT demo task.
 */
void fat_demo_entry(void *argv)
{
    uint64_t tick_bf, tick_af;
    uint32_t i;
    int32_t bytes;
    FD fd;

    UNUSED_PARAM(argv);

    /* Create a write file. */
    fd = fs_open("\\fatfs\\0\\testfile.txt", (FS_CREATE | FS_WRITE));
    if (fd != NULL)
    {
        /* Write some data on it. */
        bytes = fs_write(fd, (uint8_t *)"test string 1", strlen("test string 1"));

        /* Close this file. */
        fs_close(&fd);
    }
    else
    {
        printf("fs_open failed.\r\n");
    }

    /* Create the created file to read back. */
    fd = fs_open("\\fatfs\\0\\testfile.txt", (FS_READ));
    if (fd != NULL)
    {
        /* Read file content. */
        bytes = fs_read(fd, test_buffer, sizeof(test_buffer) - 1);

        /* Print read data on console. */
        test_buffer[bytes] = '\0';
        printf("Read data: %s\r\n", test_buffer);

        /* Close this file. */
        fs_close(&fd);
    }
    else
    {
        printf("fs_open failed.\r\n");
    }

    /* Create a large write file. */
    fd = fs_open("\\fatfs\\0\\longfile.txt", (FS_CREATE | FS_WRITE));
    if (fd != NULL)
    {
        /* Set a predefined pattern on the test buffer. */
        memset(test_buffer, 'A', sizeof(test_buffer));

        /* Save the before tick. */
        tick_bf = current_system_tick();

        /* Write data on the file. */
        for (i = 0; i < FAT_DEMO_FILE_SIZE; i += sizeof(test_buffer))
        {
            /* Write a chunk on the file. */
            bytes = fs_write(fd, test_buffer, sizeof(test_buffer));

            /* If write failed. */
            if (bytes <= 0)
            {
                break;
            }
        }

        /* Save the after tick. */
        tick_af = current_system_tick();

        /* Display the statistics. */
        printf("Written %lu in %lu ticks\r\n", i, (uint32_t)(tick_af - tick_bf));

        /* Close this file. */
        fs_close(&fd);
    }
    else
    {
        printf("fs_open failed.\r\n");
    }

    /* Open the large file for reading. */
    fd = fs_open("\\fatfs\\0\\longfile.txt", (FS_READ));
    if (fd != NULL)
    {
        /* Save the before tick. */
        tick_bf = current_system_tick();

        /* Read data from file. */
        for (i = 0; i < FAT_DEMO_FILE_SIZE; i += sizeof(test_buffer))
        {
            /* Read a chunk from the file. */
            bytes = fs_read(fd, test_buffer, sizeof(test_buffer));

            /* If read failed. */
            if (bytes <= 0)
            {
                break;
            }
        }

        /* Save the after tick. */
        tick_af = current_system_tick();

        /* Display the statistics. */
        printf("Read %lu in %lu ticks\r\n", i, (uint32_t)(tick_af - tick_bf));

        /* Close this file. */
        fs_close(&fd);
    }
    else
    {
        printf("fs_open failed.\r\n");
    }

    util_print_sys_info();

} /* fat_demo_entry */

/* Main entry function. */
int main(void)
{
    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize file system. */
    fs_init();

    /* Initialize serial. */
    serial_init();

    /* Initialize demo tasks. */
    task_create(&fat_demo_cb, P_STR("FAT"), fat_demo_stack, FAT_DEMO_STACK_SIZE, &fat_demo_entry, NULL, 0);
    scheduler_task_add(&fat_demo_cb, 15);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
