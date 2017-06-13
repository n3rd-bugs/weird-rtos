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
 * (in any form) the author will not be liable for any outcome from it's direct
 * or indirect use.
 */
#include <os.h>

#ifdef CONFIG_NET
#include <string.h>
#include <sll.h>
#include <net.h>

/* Exported networking buffer data. */
FD net_buff_fd = (FD)NULL;

/* Global net buffer data. */
static NET_BUFFER_FS net_buffers_fs;

/* Internal function prototypes. */
static int32_t net_buffer_lock(void *, uint32_t);
static void net_buffer_unlock(void *);
static void net_buffer_condition_callback(void *, int32_t);
static int32_t net_buffer_write(void *, uint8_t *, int32_t);
static int32_t net_buffer_read(void *, uint8_t *, int32_t);

/*
 * net_buffer_init
 * This function will initialize the buffer file system for out networking
 * stack.
 */
void net_buffer_init()
{
    SYS_LOG_FUNTION_ENTRY(NET_BUFFER);

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
    net_buffers_fs.fs.flags = (FS_SPACE_AVAILABLE);
    net_buffers_fs.fs.timeout = MAX_WAIT;

    /* Initialize file system condition. */
    fs_condition_init(&net_buffers_fs.fs);

#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect net buffer file descriptor. */
    semaphore_create(&net_buffers_fs.lock, 1, 1, FALSE);
#endif

    /* Register net buffer file system. */
    fs_register((FS *)&net_buffers_fs);

    /* Set the global networking stack buffer descriptor. */
    net_buff_fd = (FD)&net_buffers_fs;

    SYS_LOG_FUNTION_EXIT(NET_BUFFER);

} /* net_buffer_init */

/*
 * net_buffer_get_condition
 * @condition: Pointer where condition will be returned.
 * @suspend: Suspend needed to be populated.
 * @process: Pointer where process will be returned.
 * This function will return the condition to for networking buffers.
 */
void net_buffer_get_condition(CONDITION **condition, SUSPEND *suspend, NET_CONDITION_PROCESS **process)
{
    SYS_LOG_FUNTION_ENTRY(NET_BUFFER);

    /* For networking buffers we will wait for data on networking buffer file descriptor. */
    fs_condition_get((FD)&net_buffers_fs, condition, suspend, &net_buffers_fs.fs_param, FS_BLOCK_READ);

    /* Set callback that is needed to be called when this condition is fulfilled. */
    *process = &net_buffer_condition_callback;

    SYS_LOG_FUNTION_EXIT(NET_BUFFER);

} /* net_buffer_get_condition */

/*
 * net_buffer_condition_callback
 * @data: Unused parameter.
 * @status: Resumption status.
 * Function that will be called when networking condition is valid.
 */
static void net_buffer_condition_callback(void *data, int32_t status)
{
    FS_BUFFER *buffer;
    FD buffer_fd;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(status);

    /* Remove a compiler warning. */
    UNUSED_PARAM(data);

    SYS_LOG_FUNTION_ENTRY(NET_BUFFER);

    /* Read a buffer pointer from the file descriptor. */
    if (fs_read(net_buff_fd, (uint8_t *)&buffer, sizeof(FS_BUFFER *)) == sizeof(FS_BUFFER *))
    {
        /* Save the file descriptor on which data was received. */
        buffer_fd = buffer->fd;

        /* TODO: This is quite expensive. */
        /* Obtain lock for the file descriptor on which this semaphore was
         * received, this is required as the buffer will return it's
         * segments to the original file descriptor when applicable. */
        OS_ASSERT(fd_get_lock(buffer_fd) != SUCCESS);

        /* Process this buffer. */
        if (net_buffer_process(buffer) != NET_BUFFER_CONSUMED)
        {
            /* Free this buffer. */
            fs_buffer_add_buffer_list(buffer, FS_BUFFER_LIST, FS_BUFFER_ACTIVE);
        }

        /* Release semaphore for the buffer. */
        fd_release_lock(buffer_fd);
    }

    SYS_LOG_FUNTION_EXIT(NET_BUFFER);

} /* net_buffer_condition_callback */

/*
 * net_buffer_lock
 * @fd: Networking buffer file descriptor.
 * @timeout: Number of ticks we need to wait for the lock.
 * This function will get the lock for net buffer file descriptor.
 */
static int32_t net_buffer_lock(void *fd, uint32_t timeout)
{
    int32_t status = SUCCESS;

    SYS_LOG_FUNTION_ENTRY(NET_BUFFER);

#ifdef CONFIG_SEMAPHORE
    /* Obtain data lock for networking buffers. */
    status = semaphore_obtain(&((NET_BUFFER_FS *)fd)->lock, timeout);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(timeout);

    /* Lock scheduler. */
    scheduler_lock();
#endif

    SYS_LOG_FUNTION_EXIT_STATUS(NET_BUFFER, status);

    /* Return status to the caller. */
    return (status);

} /* net_buffer_lock */

/*
 * net_buffer_unlock
 * @fd: File descriptor for the networking buffer.
 * This function will release the lock for net buffer file descriptor.
 */
static void net_buffer_unlock(void *fd)
{
    SYS_LOG_FUNTION_ENTRY(NET_BUFFER);

#ifdef CONFIG_SEMAPHORE
    /* Release data lock for networking buffers. */
    semaphore_release(&((NET_BUFFER_FS *)fd)->lock);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Enable scheduling. */
    scheduler_unlock();
#endif

    SYS_LOG_FUNTION_EXIT(NET_BUFFER);

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
static int32_t net_buffer_write(void *fd, uint8_t *data, int32_t nbytes)
{
    NET_BUFFER_FS *net_buffer = (NET_BUFFER_FS *)fd;

    /* Unused parameter. */
    UNUSED_PARAM(nbytes);

    SYS_LOG_FUNTION_ENTRY(NET_BUFFER);

    /* Caller already has the lock for net buffer data. */

    /* Push the buffer on the network buffer queue. */
    sll_append(&net_buffer->buffer_list, (FS_BUFFER *)data, OFFSETOF(FS_BUFFER, next));

    /* Tell the file system that there is some data available on this file descriptor. */
    fd_data_available(fd);

    SYS_LOG_FUNTION_EXIT(NET_BUFFER);

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
static int32_t net_buffer_read(void *fd, uint8_t *buffer, int32_t size)
{
    NET_BUFFER_FS *net_buffer = (NET_BUFFER_FS *)fd;
    FS_BUFFER *fs_buffer;
    int32_t nbytes = sizeof(FS_BUFFER *);

    /* Unused parameter. */
    UNUSED_PARAM(size);

    SYS_LOG_FUNTION_ENTRY(NET_BUFFER);

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
        /* Remove the buffer link. */
        fs_buffer->next = NULL;

        /* Return the buffer to the caller. */
        *((FS_BUFFER **)buffer) = fs_buffer;
    }

    /* If are not returning any data or the list is now empty. */
    if ((fs_buffer == NULL) || (net_buffer->buffer_list.head == NULL))
    {
        /* Tell the file system that there is no data on the file descriptor. */
        fd_data_flushed(fd);
    }

    SYS_LOG_FUNTION_EXIT(NET_BUFFER);

    /* Return the number of bytes. */
    return (nbytes);

} /* net_buffer_read */

#endif /* CONFIG_NET */
