/*
 * fs_buffer_test.c
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

/* Demo configurations. */
#define DEMO_STACK_SIZE     512
#define TEST_NUM_BUFFER     10
#define TEST_BUFFER_SIZE    3

/* Demo task stack. */
uint8_t fs_buffer_test_stack[DEMO_STACK_SIZE];

/* Dummy file descriptor. */
FS              test_fd;
FS_BUFFER_DATA  test_buffer_data;
FS_BUFFER_ONE   test_buffers[TEST_NUM_BUFFER];
FS_BUFFER       test_buffer_list;
uint8_t         test_buffer[TEST_BUFFER_SIZE * TEST_NUM_BUFFER];
uint8_t         test_cmp_buffer[TEST_BUFFER_SIZE * TEST_NUM_BUFFER];

/* Test data must have more bytes than the (TEST_NUM_BUFFER * TEST_BUFFER_SIZE). */
const uint8_t   *test_data = (uint8_t *)"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

/* Function prototypes. */
void fs_buffer_test_task(void *);

void fs_buffer_test_task(void *argv)
{
    FD fd = (FD)&test_fd;
    uint8_t i;
    FS_BUFFER *buffer;

    memset(&test_fd, 0, sizeof(FS));
    memset(&test_buffer_data, 0, sizeof(FS_BUFFER_DATA));
    memset(&test_buffer_list, 0, sizeof(FS_BUFFER));
    memset(test_buffers, 0, (sizeof(FS_BUFFER_ONE) * 10));
    memset(test_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
    memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));

    /* Some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Set buffer data for our file descriptor. */
    fs_buffer_dataset(fd, &test_buffer_data, TEST_NUM_BUFFER);

    /* Push the one buffers on the file descriptor. */
    for (i = 0; i < TEST_NUM_BUFFER; i++)
    {
        /* Initialize a buffer. */
        fs_buffer_one_init(&test_buffers[i], &test_buffer[TEST_BUFFER_SIZE * i], TEST_BUFFER_SIZE);

        /* Add this buffer to the free buffer list for this file descriptor. */
        fs_buffer_add(fd, &test_buffers[i], FS_BUFFER_ONE_FREE, FS_BUFFER_ACTIVE);
    }

    /* Initialize a buffer. */
    fs_buffer_init(&test_buffer_list, fd);

    /* Add a buffer to the free buffer list for this file descriptor. */
    fs_buffer_add(fd, &test_buffer_list, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);

    /* Pull a buffer from this file descriptor. */
    buffer = fs_buffer_get(fd, FS_BUFFER_LIST, 0);
    ASSERT(buffer == NULL);

    /* Push test data on the buffer normally. */
    fs_buffer_push(buffer, (uint8_t *)test_data, (TEST_NUM_BUFFER * TEST_BUFFER_SIZE), 0);

    /* Pull all the data that we pushed on this buffer. */
    memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
    fs_buffer_pull(buffer, test_cmp_buffer, buffer->total_length, 0);

    /* Check if we have anticipated data. */
    ASSERT(memcmp(test_cmp_buffer, test_data, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER)) != 0);

    /* Push test data on the buffer on head. */
    fs_buffer_push(buffer, (uint8_t *)test_data, (TEST_NUM_BUFFER * TEST_BUFFER_SIZE), FS_BUFFER_HEAD);

    /* Pull all the data that we pushed on this buffer. */
    memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
    fs_buffer_pull(buffer, test_cmp_buffer, buffer->total_length, 0);

    /* Check if we have anticipated data. */
    ASSERT(memcmp(test_cmp_buffer, test_data, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER)) != 0);

    /* Push test data on the buffer but treat it as packed. */
    fs_buffer_push(buffer, (uint8_t *)test_data, (TEST_NUM_BUFFER * TEST_BUFFER_SIZE), FS_BUFFER_PACKED);

    /* Pull all the data that we pushed on this buffer. */
    memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
    fs_buffer_pull(buffer, test_cmp_buffer, buffer->total_length, FS_BUFFER_PACKED);

    /* Check if we have anticipated data. */
    ASSERT(memcmp(test_cmp_buffer, test_data, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER)) != 0);

    /* Push test data on the buffer but treat it as packed and used head scheme. */
    fs_buffer_push(buffer, (uint8_t *)test_data, (TEST_NUM_BUFFER * TEST_BUFFER_SIZE), (FS_BUFFER_PACKED | FS_BUFFER_HEAD));

    /* Pull all the data that we pushed on this buffer. */
    memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
    fs_buffer_pull(buffer, test_cmp_buffer, buffer->total_length, FS_BUFFER_PACKED);

    /* Check if we have anticipated data. */
    ASSERT(memcmp(test_cmp_buffer, test_data, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER)) != 0);

    /* Push test data on the buffer on head. */
    fs_buffer_push(buffer, (uint8_t *)test_data, (TEST_NUM_BUFFER * TEST_BUFFER_SIZE), FS_BUFFER_HEAD);

    /* Pull all the data that we pushed in this buffer. */
    memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
    fs_buffer_pull(buffer, test_cmp_buffer, buffer->total_length, (FS_BUFFER_INPLACE));

    /* Check if we have anticipated data. */
    ASSERT(memcmp(test_cmp_buffer, test_data, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER)) != 0);

    for (i = 0; i < (TEST_BUFFER_SIZE * TEST_NUM_BUFFER); i ++)
    {
        /* Push test data on the buffer on head with an offset. */
        fs_buffer_push_offset(buffer, (uint8_t *)(test_data + i), (uint32_t)((TEST_NUM_BUFFER * TEST_BUFFER_SIZE) - i), i, (FS_BUFFER_HEAD | FS_BUFFER_UPDATE));

        /* Pull all the data that we pushed in this buffer. */
        memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
        fs_buffer_pull(buffer, test_cmp_buffer, buffer->total_length, (FS_BUFFER_INPLACE));

        /* Check if we have anticipated data. */
        ASSERT(memcmp(test_cmp_buffer, test_data, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER)) != 0);
    }

    /* Pull all the data that we pushed on this buffer. */
    memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
    fs_buffer_pull(buffer, test_cmp_buffer, buffer->total_length, 0);

    /* Check if we have anticipated data. */
    ASSERT(memcmp(test_cmp_buffer, test_data, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER)) != 0);

    /* Push test data on the buffer on head as packet. */
    fs_buffer_push(buffer, (uint8_t *)test_data, (TEST_NUM_BUFFER * TEST_BUFFER_SIZE), (FS_BUFFER_PACKED | FS_BUFFER_HEAD));

    /* Pull all the data that we pushed in this buffer. */
    memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
    fs_buffer_pull(buffer, test_cmp_buffer, buffer->total_length, (FS_BUFFER_PACKED | FS_BUFFER_INPLACE));

    /* Check if we have anticipated data. */
    ASSERT(memcmp(test_cmp_buffer, test_data, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER)) != 0);

    for (i = 1; i < (TEST_BUFFER_SIZE * TEST_NUM_BUFFER); i ++)
    {
        /* Push test data on the buffer on head with an offset. */
        fs_buffer_push_offset(buffer, (uint8_t *)test_data, (uint32_t)((TEST_NUM_BUFFER * TEST_BUFFER_SIZE) - i), i, (FS_BUFFER_PACKED | FS_BUFFER_HEAD | FS_BUFFER_UPDATE));

        /* Pull all the data that we pushed in this buffer. */
        memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
        fs_buffer_pull(buffer, test_cmp_buffer, buffer->total_length, (FS_BUFFER_PACKED | FS_BUFFER_INPLACE));

        /* Check if we have anticipated data. */
        ASSERT(memcmp(test_cmp_buffer, test_data, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER)) != 0);
    }

    /* Pull all the data that we pushed on this buffer. */
    memset(test_cmp_buffer, 0, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER));
    fs_buffer_pull(buffer, test_cmp_buffer, buffer->total_length, (FS_BUFFER_PACKED));

    /* Check if we have anticipated data. */
    ASSERT(memcmp(test_cmp_buffer, test_data, (TEST_BUFFER_SIZE * TEST_NUM_BUFFER)) != 0);

}

int main(void)
{
    TASK fs_buffer_test_cb;

    /* Initialize scheduler. */
    scheduler_init();

    /* Initialize file system. */
    fs_init();

    /* Create a task to test file system buffer functionality. */
    task_create(&fs_buffer_test_cb, P_STR("ECHO"), fs_buffer_test_stack, DEMO_STACK_SIZE, &fs_buffer_test_task, (void *)(NULL), 0);
    scheduler_task_add(&fs_buffer_test_cb, 5);

    /* Run scheduler. */
    kernel_run();

    return (0);

}
