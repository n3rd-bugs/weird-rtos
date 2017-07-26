/*
 * hdr_parser.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
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
#include <mem.h>
#include <header.h>
#include <fs.h>

/* Function prototypes. */
void parser_demo_task(void *);
int32_t process_result(void *, uint8_t *, uint32_t);

/* IPv4 header list. */
const HEADER dummy_hdr[] =
{
    /*  Header data         Size,   Type flags,                 */
    {   &process_result,    2,      HEADER_BIT|HEADER_PROCESS,  },
    {   &process_result,    5,      HEADER_BIT|HEADER_PROCESS,  },
    {   &process_result,    1,      HEADER_BIT|HEADER_PROCESS,  },
    {   &process_result,    4,      HEADER_BIT|HEADER_PROCESS,  },
    {   &process_result,    4,      HEADER_BIT|HEADER_PROCESS,  },
    {   &process_result,    0,      HEADER_END,                 },
};

uint8_t dummy_data[] = { 0xAB, 0xAB, 0xBC, 0x78, 0x11, 0x67 };
uint8_t proc_buffer[5];

int32_t process_result(void *data, uint8_t *value, uint32_t length)
{
    UNUSED_PARAM(data);
    UNUSED_PARAM(value);
    UNUSED_PARAM(length);

    /* Always return success. */
    return (SUCCESS);

} /* process_result */

void parser_demo_task(void *argv)
{
    HDR_MACHINE machine;
    FS_BUFFER buffer;
    FS_BUFFER_ONE one;

    UNUSED_PARAM(argv);

    /* Initialize a file system buffer. */
    fs_buffer_init(&buffer, NULL);
    fs_buffer_one_init(&one, (uint8_t *)dummy_data, sizeof(dummy_data));
    one.length = sizeof(dummy_data);
    fs_buffer_add_one(&buffer, &one, 0);

    /* Initialize header machine. */
    header_machine_init(&machine, &fs_buffer_hdr_pull);

    /* Parse the header. */
    header_machine_run(&machine, NULL, dummy_hdr, &buffer, proc_buffer);

}

int main(void)
{
    TASK *task_cb;

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize memory. */
    mem_init();

    /* Initialize file system. */
    fs_init();

    /* Create a task for CDC demo. */
    task_cb = (TASK *)mem_static_alloc(sizeof(TASK) + 4096);
    task_create(task_cb, P_STR("STATS"), (uint8_t *)(task_cb + 1), 4096, &parser_demo_task, (void *)(NULL), 0);
    scheduler_task_add(task_cb, 5);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
