/*
 * net_buffer.c
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

#ifdef CONFIG_NET
#include <string.h>
#include <sll.h>
#include <net.h>

/* Internal function prototypes. */
static int32_t net_buffer_lock(void *);
static void net_buffer_unlock(void *);
static void net_buffer_receive_task_entry(void *);
static int32_t net_buffer_write(void *, char *, int32_t);
static int32_t net_buffer_read(void *, char *, int32_t);

/* Global net buffer data. */
NET_BUFFER_FS net_buffers_fs;

/* Exported networking file descriptor. */
FD net_buff_fd = (FD)NULL;

/* Net buffer receive task data */
TASK net_buffer_receive_tcb;
char net_buffer_receive_stack[NET_BUFFER_RX_STACK_SIZE];

/*
 * net_buffer_init
 * This function will initialize the buffer file system for out networking
 * stack.
 */
void net_buffer_init()
{
    /* Clear the global structure. */
    memset(&net_buffers_fs, 0, sizeof(NET_BUFFER_FS));

    /* Initialize the buffer data. */
    net_buffers_fs.fs.name = "\\net\\buffers";
    net_buffers_fs.fs.get_lock = &net_buffer_lock;
    net_buffers_fs.fs.release_lock = &net_buffer_unlock;

    /* Read and write functions. */
    net_buffers_fs.fs.write = &net_buffer_write;
    net_buffers_fs.fs.read = &net_buffer_read;

    /* Initial file system configurations. */
    net_buffers_fs.fs.flags = (FS_SPACE_AVAILABLE | FS_BLOCK);
    net_buffers_fs.fs.timeout = MAX_WAIT;

#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect net buffer file descriptor. */
    semaphore_create(&net_buffers_fs.lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

    /* Register net buffer file system. */
    fs_register((FS *)&net_buffers_fs);

    /* Create a task to process the incoming networking buffers. */
    task_create(&net_buffer_receive_tcb, "NET-RX", net_buffer_receive_stack, NET_BUFFER_RX_STACK_SIZE, &net_buffer_receive_task_entry, (void *)(&net_buffers_fs), TASK_NO_RETURN);
    scheduler_task_add(&net_buffer_receive_tcb, TASK_APERIODIC, 5, 0);

} /* net_buffer_init */

/*
 * net_buffer_receive
 * @argv: Net buffer file descriptor on which we will be listening for data.
 * This is task entry function for receiving and processing the incoming
 * networking buffers.
 */
static void net_buffer_receive_task_entry(void *argv)
{
    FS_BUFFER *buffer;

    /* Set the global file descriptor for net buffers. */
    net_buff_fd = (FD)argv;

    /* This function should never return. */
    for (;;)
    {
        /* Read a buffer pointer from the file descriptor. */
        if (fs_read(net_buff_fd, (char *)(&buffer), sizeof(FS_BUFFER *)) == sizeof(FS_BUFFER *))
        {
            /* Process this buffer. */
            if (net_buffer_process(buffer) == SUCCESS)
            {
                /* Free this buffer. */
                fs_buffer_add(buffer->fd, buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
            }
        }
    }

} /* net_buffer_receive */

/*
 * net_buffer_lock
 * @fd: Net buffer file descriptor.
 * This function will get the lock for net buffer file descriptor.
 */
static int32_t net_buffer_lock(void *fd)
{
#ifdef CONFIG_SEMAPHORE
    /* Obtain data lock for networking buffers. */
    return semaphore_obtain(&((NET_BUFFER_FS *)fd)->lock, MAX_WAIT);
#else
    /* Lock scheduler. */
    scheduler_lock();

    /* Return success. */
    return (SUCCESS);
#endif
} /* net_buffer_lock */

/*
 * net_buffer_unlock
 * @fd: File descriptor for the console.
 * This function will release the lock for net buffer file descriptor.
 */
static void net_buffer_unlock(void *fd)
{
#ifdef CONFIG_SEMAPHORE
    /* Release data lock for networking buffers. */
    semaphore_release(&((NET_BUFFER_FS *)fd)->lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif
} /* net_buffer_unlock */

/*
 * net_buffer_write
 * @fd: File descriptor.
 * @data: Buffer pointer needed to be written on this descriptor.
 * @nbytes: Number of bytes, should always be size of a pointer.
 * @return: Number of bytes written.
 * This function will write a networking buffer and queue it for further
 * processing.
 */
static int32_t net_buffer_write(void *fd, char *data, int32_t nbytes)
{
    NET_BUFFER_FS *net_buffer = (NET_BUFFER_FS *)fd;

    /* Unused parameter. */
    UNUSED_PARAM(nbytes);

    /* Caller already has the lock for net buffer data. */

    /* Push the buffer on the network buffer queue. */
    sll_append(&net_buffer->buffer_list, (FS_BUFFER *)data, OFFSETOF(FS_BUFFER, next));

    /* Tell the file system that there is some data available on this file descriptor. */
    fd_data_available(fd);

    /* Return the number of bytes. */
    return (sizeof(FS_BUFFER *));

} /* net_buffer_write */

/*
 * net_buffer_read
 * @fd: File descriptor.
 * @buffer: Pointer to a buffer pointer that will be updated with the available
 *  buffer.
 * @size: Size of buffer, should be the size of a pointer.
 * @return: Number of bytes read.
 * This function will dequeue a networking buffer and return it's pointer in
 * the provided buffer.
 */
static int32_t net_buffer_read(void *fd, char *buffer, int32_t size)
{
    NET_BUFFER_FS *net_buffer = (NET_BUFFER_FS *)fd;
    FS_BUFFER *fs_buffer;
    int32_t nbytes = sizeof(FS_BUFFER *);

    /* Unused parameter. */
    UNUSED_PARAM(size);

    /* Caller already has the lock for net buffer data. */

    /* Pull a networking buffer from the list. */
    fs_buffer = sll_pop(&net_buffer->buffer_list, OFFSETOF(FS_BUFFER, next));

    /* If there is no buffer on the list to process. */
    if (fs_buffer == NULL)
    {
        /* We did not get any data. */
        nbytes = 0;
    }
    else
    {
        /* Return the buffer to the caller. */
        *((FS_BUFFER **)buffer) = fs_buffer;
    }

    /* If are not returning any data or the list is now empty. */
    if ((fs_buffer == NULL) || (net_buffer->buffer_list.head == NULL))
    {
        /* Tell the file system that there is no data on the file descriptor. */
        fd_data_flushed(fd);
    }

    /* Return the number of bytes. */
    return (nbytes);

} /* net_buffer_read */

#endif /* CONFIG_NET */
