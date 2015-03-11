/*
 * fs.c
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
#include <sll.h>
#include <path.h>
#include <sleep.h>

#ifdef CONFIG_FS

/* Global variables. */
static FS_DATA file_data;

/*
 * fs_init
 * This function will initialize file system layer. This function must be
 * called before using any other APIs.
 */
void fs_init()
{
    /* Clear the global file system data. */
    memset(&file_data, 0, sizeof(FS_DATA));

#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect global file system data. */
    semaphore_create(&file_data.lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

#ifdef FS_CONSOLE
    /* Initialize console. */
    console_init();
#endif

#ifdef FS_PIPE
    /* Initialize PIPE file system. */
    pipe_init();
#endif

} /* fs_init */

/*
 * fs_register
 * @file_system: File system data to be registered.
 * This function registers a file system, this is called by lower layer to
 * register a new file system.
 */
void fs_register(FS *file_system)
{
#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Obtain the global data lock. */
    OS_ASSERT(semaphore_obtain(&file_data.lock, MAX_WAIT) != SUCCESS);
#endif

    /* Just push this file system in the list. */
    sll_push(&file_data.list, file_system, OFFSETOF(FS, next));

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&file_data.lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif

} /* fs_register */

/*
 * fs_unregister
 * @file_system: File system data needed to unregistered.
 * This function will unregister a file system. Must be called by the
 * component that has registered the file system.
 * Note that the invalidating a file descriptor is needed to be maintained by
 * underlying layer.
 */
void fs_unregister(FS *file_system)
{
#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Obtain the global data lock. */
    OS_ASSERT(semaphore_obtain(&file_data.lock, MAX_WAIT) != SUCCESS);
#endif

    /* This could be a file descriptor chain, so destroy it. */
    fs_destroy_chain((FD)file_system);

    /* Just remove this file system from the list. */
    OS_ASSERT(sll_remove(&file_data.list, file_system, OFFSETOF(FS, next)) != file_system);

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&file_data.lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif

} /* fs_unregister */

/*
 * fs_init_buffer
 * @buffer: File buffer needed to be initialized.
 * @data: Data space needed to be used for this buffer.
 * @size: Size of the data used with buffer.
 * This function will initialize a buffer with given data.
 */
void fs_init_buffer(FS_BUFFER *buffer, char *data, uint32_t size)
{
    /* Clear this buffer. */
    memset(buffer, 0, sizeof(FS_BUFFER));

    /* Initialize this buffer. */
    buffer->init_buffer = data;
    buffer->init_length = size;

} /* fs_init_buffer */

/*
 * fs_update_buffer
 * @buffer: File buffer needed to be initialized.
 * @data: New buffer pointer.
 * @size: Size valid data in the buffer.
 * This function will update a buffer data pointers.
 */
void fs_update_buffer(FS_BUFFER *buffer, char *data, uint32_t size)
{
    /* Update the buffer data. */
    buffer->buffer = data;
    buffer->length = size;

} /* fs_update_buffer */

/*
 * fs_add_buffer
 * @fd: File descriptor on which a free buffer is needed to be added.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_FREE: if this is a free buffer.
 *  FS_BUFFER_RX: if this is a receive buffer.
 *  FS_BUFFER_TX: if this is a transmit buffer.
 * @flags: Operation flags.
 *  FS_BUFFER_ACTIVE: actively add the buffer and invoke the any callbacks.
 * This function will add a new buffer in the file descriptor for the required
 * type.
 */
void fs_add_buffer(FD fd, FS_BUFFER *buffer, uint32_t type, uint32_t flags)
{
    /* Type of buffer we are adding. */
    switch (type)
    {
    case (FS_BUFFER_FREE):

        /* Initialize this buffer. */
        buffer->buffer = buffer->init_buffer;
        buffer->length = buffer->init_length;

        /* Just add this buffer in the free buffer list. */
        sll_append(&((FS *)fd)->free_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));

        /* If we are doing this actively. */
        if (flags & FS_BUFFER_ACTIVE)
        {
            /* Some space is now available. */
            fd_space_available(fd);
        }

        break;

    case (FS_BUFFER_RX):

        /* Just add this buffer in the receive buffer list. */
        sll_append(&((FS *)fd)->rx_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));

        /* If we are doing this actively. */
        if (flags & FS_BUFFER_ACTIVE)
        {
            /* Some new data is now available. */
            fd_data_available(fd);
        }

        break;

    case (FS_BUFFER_TX):

        /* Just add this buffer in the transmit buffer list. */
        sll_append(&((FS *)fd)->tx_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));

        break;
    }

} /* fs_add_buffer */

/*
 * fs_get_buffer
 * @fd: File descriptor from which a free buffer is needed.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_FREE: if this is a free buffer.
 *  FS_BUFFER_RX: if this is a receive buffer.
 *  FS_BUFFER_TX: if this is a transmit buffer.
 * @flags: Operation flags.
 *  FS_BUFFER_ACTIVE: actively pickup the buffer and invoke the any callbacks.
 *  FS_BUFFER_INPLACE: will not remove the buffer from the list just return a
 *      pointer to it.
 * This function return a buffer from a required buffer list for this file
 * descriptor.
 */
FS_BUFFER *fs_get_buffer(FD fd, uint32_t type, uint32_t flags)
{
    FS_BUFFER *buffer = NULL;

    /* Type of buffer we need. */
    switch (type)
    {
    case (FS_BUFFER_FREE):

        /* If we need to return the buffer in-place. */
        if (flags & FS_BUFFER_INPLACE)
        {
            /* Return the pointer to the head buffer. */
            buffer = ((FS *)fd)->free_buffer_list.head;
        }
        else
        {
            /* Pop a buffer from this file descriptor's free buffer list. */
            buffer = sll_pop(&((FS *)fd)->free_buffer_list, OFFSETOF(FS_BUFFER, next));

            /* If we are doing this actively. */
            if ((flags & FS_BUFFER_ACTIVE) &&
                (((FS *)fd)->free_buffer_list.head == NULL))
            {
                /* Tell the file system to block the write until there is some
                 * space available. */
                fd_space_consumed(fd);
            }
        }

        break;

    case (FS_BUFFER_RX):

        /* If we need to return the buffer in-place. */
        if (flags & FS_BUFFER_INPLACE)
        {
            /* Return the pointer to the head buffer. */
            buffer = ((FS *)fd)->rx_buffer_list.head;
        }
        else
        {
            /* Pop a buffer from this file descriptor's receive buffer list. */
            buffer = sll_pop(&((FS *)fd)->rx_buffer_list, OFFSETOF(FS_BUFFER, next));

            /* If we are doing this actively. */
            if ((flags & FS_BUFFER_ACTIVE) &&
                (((FS *)fd)->rx_buffer_list.head == NULL))
            {
                /* No more data is available to read. */
                fd_data_flushed(fd);
            }
        }

        break;

    case (FS_BUFFER_TX):

        /* If we need to return the buffer in-place. */
        if (flags & FS_BUFFER_INPLACE)
        {
            /* Return the pointer to the head buffer. */
            buffer = ((FS *)fd)->tx_buffer_list.head;
        }
        else
        {
            /* Pop a buffer from this file descriptor's transmit buffer list. */
            buffer = sll_pop(&((FS *)fd)->tx_buffer_list, OFFSETOF(FS_BUFFER, next));
        }

        break;
    }

    /* Return the buffer. */
    return (buffer);

} /* fs_get_buffer */

/*
 * fs_set_data_watcher
 * @fd: File descriptor for which data is needed to be monitored.
 * @watcher: Data watcher needed to be registered.
 * This function will add a data watcher for the given file system.
 */
void fs_set_data_watcher(FD fd, FS_DATA_WATCHER *watcher)
{
    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        OS_ASSERT(((FS *)fd)->get_lock((void *)fd) != SUCCESS);
    }

    /* Add this watcher is the watcher list. */
    sll_append(&((FS *)fd)->data_watcher_list, watcher, OFFSETOF(FS_DATA_WATCHER, next));

    if (((FS *)fd)->release_lock)
    {
        /* Release lock for this file descriptor. */
        ((FS *)fd)->release_lock((void *)fd);
    }

} /* fs_set_connection_watcher */

/*
 * fs_set_connection_watcher
 * @fd: File descriptor for which connection is needed to be monitored.
 * @watcher: Connection watcher needed to be registered.
 * This function will add a connection watcher for given file system.
 */
void fs_set_connection_watcher(FD fd, FS_CONNECTION_WATCHER *watcher)
{
    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        OS_ASSERT(((FS *)fd)->get_lock((void *)fd) != SUCCESS);
    }

    /* Add this watcher is the watcher list. */
    sll_append(&((FS *)fd)->connection_watcher_list, watcher, OFFSETOF(FS_CONNECTION_WATCHER, next));

    if (((FS *)fd)->release_lock)
    {
        /* Release lock for this file descriptor. */
        ((FS *)fd)->release_lock((void *)fd);
    }

} /* fs_set_connection_watcher */

/*
 * fs_connected
 * @fd: File descriptor for which connection was established.
 * This function will be called by driver when this console is connected.
 */
void fs_connected(FD fd)
{
    FS_CONNECTION_WATCHER *watcher;

    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        OS_ASSERT(((FS *)fd)->get_lock((void *)fd) != SUCCESS);
    }

    /* Pick the first watcher data. */
    watcher = ((FS *)fd)->connection_watcher_list.head;

    /* While we have a watcher to process. */
    while (watcher != NULL)
    {
        /* If we have a connection established watcher. */
        if (watcher->connected != NULL)
        {
            /* Call the watcher function. */
            watcher->connected(fd, watcher->data);
        }

        /* Pick the next watcher. */
        watcher = watcher->next;
    }

    if (((FS *)fd)->release_lock)
    {
        /* Release lock for this file descriptor. */
        ((FS *)fd)->release_lock((void *)fd);
    }

} /* console_connected */

/*
 * fs_disconnected
 * @fd: File descriptor for which connection was terminated.
 * This function will be called by driver when this console is disconnected.
 */
void fs_disconnected(FD fd)
{
    FS_CONNECTION_WATCHER *watcher;

    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        OS_ASSERT(((FS *)fd)->get_lock((void *)fd) != SUCCESS);
    }

    /* Pick the first watcher data. */
    watcher = ((FS *)fd)->connection_watcher_list.head;

    /* While we have a watcher to process. */
    while (watcher != NULL)
    {
        /* If we have a connection terminated watcher. */
        if (watcher->disconnected != NULL)
        {
            /* Call the watcher function. */
            watcher->disconnected(fd, watcher->data);
        }

        /* Pick the next watcher. */
        watcher = watcher->next;
    }

    if (((FS *)fd)->release_lock)
    {
        /* Release lock for this file descriptor. */
        ((FS *)fd)->release_lock((void *)fd);
    }

} /* console_disconnected */

/*
 * fs_connect
 * @fd: File descriptor needed to be connected.
 * @fd_head: Descriptor that will be defined as a head file descriptor.
 * This function will connect a file descriptor to another file descriptor that
 * will act as chain's head.
 */
void fs_connect(FD fd, FD fd_head)
{
    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        OS_ASSERT(((FS *)fd)->get_lock((void *)fd) != SUCCESS);
    }

    /* Given file descriptor should not be a chain head. */
    OS_ASSERT(((FS *)fd)->flags & FS_CHAIN_HEAD);

    /* Given file descriptor should not be a part of a chain. */
    OS_ASSERT(((FS *)fd)->fd_chain.fd_node.head != 0);

    /* Save the list head. */
    ((FS *)fd)->fd_chain.fd_node.head = fd_head;

    if (((FS *)fd)->release_lock)
    {
        /* Release lock for this file descriptor. */
        ((FS *)fd)->release_lock((void *)fd);
    }

    if (((FS *)fd_head)->get_lock)
    {
        /* Get lock for head file descriptor. */
        OS_ASSERT(((FS *)fd_head)->get_lock((void *)fd_head) != SUCCESS);
    }

    /* If head file descriptor is not a chain head. */
    if (!(((FS *)fd_head)->flags & FS_CHAIN_HEAD))
    {
        /* Initialize chain head. */
        ((FS *)fd_head)->flags |= FS_CHAIN_HEAD;
    }

    /* Push this node on the list head. */
    sll_push(&((FS *)fd_head)->fd_chain.fd_list, fd, OFFSETOF(FS, fd_chain.fd_node.next));

    if (((FS *)fd_head)->release_lock)
    {
        /* Release lock for head file descriptor. */
        ((FS *)fd_head)->release_lock((void *)fd_head);
    }

} /* fs_connect */

/*
 * fs_destroy_chain
 * @fd: File descriptor chain needed to be destroyed.
 * This function will destroy a descriptor chain. All file descriptors are
 * capable of becoming a chain head so this function should be called when a
 * file descriptor is being destroyed.
 */
void fs_destroy_chain(FD fd)
{
    FS *fs;

    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        OS_ASSERT(((FS *)fd)->get_lock((void *)fd) != SUCCESS);
    }

    /* Get a file descriptor from list. */
    fs = (FS *)sll_pop(&((FS *)fd)->fd_chain.fd_list, OFFSETOF(FS, fd_chain.fd_node.next));

    /* While we have file descriptor in the chain. */
    while (fs != NULL)
    {
        if (fs->get_lock)
        {
            /* Get lock for head file descriptor. */
            OS_ASSERT(fs->get_lock((void *)fs) != SUCCESS);
        }

        /* Remove this file descriptor from the list. */
        fs->fd_chain.fd_node.head = NULL;

        if (fs->release_lock)
        {
            /* Release lock for head file descriptor. */
            fs->release_lock((void *)fs);
        }

        /* Get next file descriptor. */
        fs = (FS *)sll_pop(&((FS *)fd)->fd_chain.fd_list, OFFSETOF(FS, fd_chain.fd_node.next));
    }

    if (((FS *)fd)->release_lock)
    {
        /* Release lock for this file descriptor. */
        ((FS *)fd)->release_lock((void *)fd);
    }

} /* fs_destroy_chain */

/*
 * fs_disconnect
 * @fd: File descriptor needed to be disconnected.
 * This function will disconnect a file descriptor from it's head.
 */
void fs_disconnect(FD fd)
{
    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        OS_ASSERT(((FS *)fd)->get_lock((void *)fd) != SUCCESS);
    }

    /* Given file descriptor should not be a chain head. */
    OS_ASSERT(((FS *)fd)->flags & FS_CHAIN_HEAD);

    /* If we are part of a file descriptor chain. */
    if (((FS *)fd)->fd_chain.fd_node.head != NULL)
    {
        if (((FS *)((FS *)fd)->fd_chain.fd_node.head)->get_lock)
        {
            /* Get lock for head file descriptor. */
            OS_ASSERT(((FS *)((FS *)fd)->fd_chain.fd_node.head)->get_lock((void *)((FS *)fd)->fd_chain.fd_node.head) != SUCCESS);
        }

        /* We must have removed this file descriptor from it's list. */
        OS_ASSERT(sll_remove(&((FS *)((FS *)fd)->fd_chain.fd_node.head)->fd_chain.fd_list, fd, OFFSETOF(FS, fd_chain.fd_node.next)) != fd);

        if (((FS *)((FS *)fd)->fd_chain.fd_node.head)->release_lock)
        {
            /* Release lock for head file descriptor. */
            ((FS *)((FS *)fd)->fd_chain.fd_node.head)->release_lock((void *)((FS *)fd)->fd_chain.fd_node.head);
        }
    }

    if (((FS *)fd)->release_lock)
    {
        /* Release lock for this file descriptor. */
        ((FS *)fd)->release_lock((void *)fd);
    }

} /* fs_disconnect */

/*
 * fs_sreach_directory
 * @node: An existing file system in the list.
 * @param: Search parameter that will be updated.
 * @return: FALSE.
 * This is a search function to search a file system that should be used
 * to process a given node.
 */
uint8_t fs_sreach_directory(void *node, void *param)
{
    /* Save the required path. */
    char *path = ((DIR_PARAM *)param)->name;
    uint8_t match = FALSE;

    /* Match the file system path. */
    if (util_path_match(((FS *)node)->name, &path) == TRUE)
    {
        /* If given path was is a directory. */
        if (*path == '\\')
        {
            /* Move past the delimiter. */
            path++;
        }

        /* If this was an exact match. */
        if (*path == '\0')
        {
            /* Got an exact match. */
            match = TRUE;
        }

        /* Update the pointer until this path was matched. */
        ((DIR_PARAM *)param)->matched = path;

        /* Save this node. */
        ((DIR_PARAM *)param)->priv = node;
    }

    /* Return if we need to stop the search or need to process more. */
    return (match);

} /* fs_sreach_directory */

/*
 * fs_sreach_node
 * @node: An existing file system in the list.
 * @param: Search parameter that will be updated.
 * @return: FALSE.
 * This is a search function to search a file system that should be used
 * to process a given node.
 */
uint8_t fs_sreach_node(void *node, void *param)
{
    /* Save the required path. */
    char *path = ((NODE_PARAM *)param)->name;
    uint8_t match = FALSE;

    /* Match the file system path and this is a exact match. */
    if ( (util_path_match(((FS *)node)->name, &path) == TRUE) && (*path == '\0') )
    {
        /* Return this node. */
        ((NODE_PARAM *)param)->priv = node;

        /* Got an match. */
        match = TRUE;
    }

    /* Return if we need to stop the search or need to process more. */
    return (match);

} /* fs_sreach_node */

/*
 * fs_open
 * @name: File name to open.
 * @flags: Open flags.
 * This function opens a named node with given flags. The name should not
 * end with a '\\'.
 */
FD fs_open(char *name, uint32_t flags)
{
    DIR_PARAM param;
    FD fd = 0;

#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data lock. */
    OS_ASSERT(semaphore_obtain(&file_data.lock, MAX_WAIT) != SUCCESS);
#endif

    /* Initialize a search parameter. */
    param.name = name;
    param.matched = NULL;
    param.priv = (void *)fd;

    /* First find a file system to which this call can be forwarded. */
    sll_search(&file_data.list, NULL, fs_sreach_directory, &param, OFFSETOF(FS, next));

    /* If a node was found. */
    if (param.priv)
    {
        /* Update the name to the resolved. */
        name = param.matched;

        /* Use this FD, we will update it if required. */
        fd = (FD)param.priv;
    }

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&file_data.lock);
#endif

    if (fd != 0)
    {
        /* Check if we need to call the underlying function to get a new file
         * descriptor. */
        if (((FS *)fd)->open != NULL)
        {
            /* Call the underlying API to get the file descriptor. */
            fd = (FD)((FS *)fd)->open(name, flags);
        }
    }

    /* Return the created file descriptor. */
    return (fd);

} /* fs_open */

/*
 * fs_close
 * @fd: Pointer to file descriptor.
 * This function will close a file descriptor.
 */
void fs_close(FD *fd)
{
    /* Check if a close function was registered with this descriptor. */
    if (((FS *)(*fd))->close != NULL)
    {
        /* Transfer call to underlying API. */
        ((FS *)(*fd))->close((void **)fd);
    }

    /* Clear the file descriptor. */
    *fd = (FD)NULL;

} /* fs_close */

/*
 * fs_read
 * @fd: File descriptor from which data is needed to be read.
 * @buffer: Buffer in which data is needed to be read.
 * @nbytes: Number of bytes that can be read in the provided buffer.
 * @return: >0 number of bytes actually read in the given buffer,
 *  FS_NODE_DELETED if file node was deleted during waiting for read,
 *  FS_READ_TIMEOUT if no data was received before the given timeout.
 * This function will read data from a file descriptor.
 */
int32_t fs_read(FD fd, char *buffer, int32_t nbytes)
{
#ifdef CONFIG_SLEEP
    uint64_t last_tick = current_system_tick();
#endif
    FS_PARAM param;
    int32_t read = 0, status = SUCCESS;

    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        status = ((FS *)fd)->get_lock((void *)fd);
    }

    /* If lock was successfully obtained. */
    if (status == SUCCESS)
    {
        /* Check if a read function was registered with this descriptor */
        if ((((FS *)fd)->read != NULL))
        {
            /* Check if we need to block on read for this FS and there is no
             * data on the descriptor and we are in a task. */
            if ((!(((FS *)fd)->flags & FS_DATA_AVAILABLE)) &&
                (((FS *)fd)->flags & FS_BLOCK) &&
                (get_current_task() != NULL))
            {
                /* There is never a surety that if some data is available and
                 * picked up by a waiting task as scheduler might decide to run
                 * some other higher/same priority task and data can get consumed
                 * by it before this waiting task can get it. */
                /* This is not a bug and happen in a RTOS where different types of
                 * schedulers are present. */
                do
                {
#ifdef CONFIG_SLEEP
                    if (((FS *)fd)->timeout != (uint32_t)MAX_WAIT)
                    {
                        /* If called again compensate for the time we have already waited. */
                        ((FS *)fd)->timeout -= (uint32_t)(current_system_tick() - last_tick);

                        /* Save when we suspended last time. */
                        last_tick = current_system_tick();
                    }
#endif
                    /* Initialize suspend criteria. */
                    param.flag = FS_BLOCK_READ;

                    /* Wait for data on this file descriptor. */
                    status = fd_suspend_criteria(fd, &param, ((FS *)fd)->timeout);

                    /* If an error has occurred. */
                    if (status != SUCCESS)
                    {
                        /* Break out of this loop. */
                        break;
                    }

                } while (!(((FS *)fd)->flags & FS_DATA_AVAILABLE));
            }

            /* Check if some data is available. */
            if ((status == SUCCESS) && (((FS *)fd)->flags & FS_DATA_AVAILABLE))
            {
                /* Transfer call to underlying API. */
                read = ((FS *)fd)->read((void *)fd, buffer, nbytes);

                /* Some data is still available. */
                if (((FS *)fd)->flags & FS_DATA_AVAILABLE)
                {
                    /* Resume any task waiting on this file descriptor. */
                    fd_data_available(fd);
                }

                /* If there is some space on this file descriptor. */
                if (((FS *)fd)->flags & FS_SPACE_AVAILABLE)
                {
                    /* Resume any tasks waiting for space on this file descriptor. */
                    fd_space_available(fd);
                }
            }
        }

        if ((status != FS_NODE_DELETED) && (((FS *)fd)->release_lock))
        {
            /* Release lock for this file descriptor. */
            ((FS *)fd)->release_lock((void *)fd);
        }
    }

    /* Return number of bytes read. */
    return (read);

} /* fs_read */

/*
 * fs_write
 * @fd: File descriptor on which data is needed to be written.
 * @buffer: Data buffer needed to be sent.
 * @nbytes: Number of bytes to write. If -1 the we will use assume that given
 *  data is a null terminated string and it's length will be calculated locally.
 * @return: Returns number of bytes written, if this descriptor is part of a
 * chain average number of bytes written will be returned.
 * This function will write data on a file descriptor.
 */
int32_t fs_write(FD fd, char *buffer, int32_t nbytes)
{
#ifdef CONFIG_SLEEP
    uint64_t last_tick = current_system_tick();
#endif
    FS_PARAM param;
    int32_t status = SUCCESS, written = 0;
    int32_t n_fd = 0, nbytes_fd;
    char *buffer_start;
    FD next_fd = NULL;
    uint8_t is_list = FALSE;

    /* If this is a null terminated string. */
    if (nbytes == -1)
    {
        /* Compute the string length. */
        nbytes = (int32_t)strlen(buffer);
    }

    /* Save buffer data. */
    buffer_start = buffer;
    nbytes_fd = nbytes;

    do
    {
        /* Initialize loop variables. */
        nbytes = nbytes_fd;
        buffer = buffer_start;

        if (((FS *)fd)->get_lock)
        {
            /* Get lock for this file descriptor. */
            status = ((FS *)fd)->get_lock((void *)fd);
        }

        /* If lock was successfully obtained. */
        if (status == SUCCESS)
        {
            /* Check if a write function was registered with this descriptor. */
            if (((FS *)fd)->write != NULL)
            {
                /* If configured try to write on the descriptor until all the
                 * data is sent. */
                do
                {
                    /* If we are in a task. */
                    if ((get_current_task() != NULL) &&

                        /* Check if we can block on write for this FD and there
                         * is no space. */
                        ((((FS *)fd)->flags & FS_BLOCK) &&
                         (!(((FS *)fd)->flags & FS_SPACE_AVAILABLE))))
                    {
                        /* There is never a surety that if some space is available and
                         * consumed up by a waiting task as scheduler might decide to run
                         * some other higher/same priority task and spaced can get consumed
                         * by it before this waiting task can get it. */
                        /* This is not a bug and happen in a RTOS where different types of
                         * schedulers are present. */
                        do
                        {
#ifdef CONFIG_SLEEP
                            if (((FS *)fd)->timeout != (uint32_t)MAX_WAIT)
                            {
                                /* If called again compensate for the time we have already waited. */
                                ((FS *)fd)->timeout -= (uint32_t)(current_system_tick() - last_tick);

                                /* Save when we suspended last time. */
                                last_tick = current_system_tick();
                            }
#endif
                            /* Initialize suspend criteria. */
                            param.flag = FS_BLOCK_WRITE;

                            /* Wait for space on this file descriptor. */
                            status = fd_suspend_criteria(fd, &param, ((FS *)fd)->timeout);

                            /* If an error has occurred. */
                            if (status != SUCCESS)
                            {
                                /* Break out of this loop. */
                                break;
                            }

                        } while (!(((FS *)fd)->flags & FS_SPACE_AVAILABLE));
                    }

                    /* Check if some space is available. */
                    if ((status == SUCCESS) && (((FS *)fd)->flags & FS_SPACE_AVAILABLE))
                    {
                        /* Transfer call to underlying API. */
                        status = ((FS *)fd)->write((void *)fd, buffer, nbytes);

                        if (status <= 0)
                        {
                            break;
                        }

                        /* Decrement number of bytes remaining. */
                        nbytes -= status;
                        buffer += status;
                        written += status;
                    }

                    /* If an error has occurred. */
                    else if (status != SUCCESS)
                    {
                        /* Break out of this loop. */
                        break;
                    }

                } while ((((FS *)fd)->flags & FS_FLUSH_WRITE) && (nbytes > 0));

                /* Some data is available. */
                if (((FS *)fd)->flags & FS_DATA_AVAILABLE)
                {
                    /* Resume any task waiting on this file descriptor. */
                    fd_data_available(fd);
                }

                /* Some space is still available. */
                if (((FS *)fd)->flags & FS_SPACE_AVAILABLE)
                {
                    /* Resume any tasks waiting for space on this file descriptor. */
                    fd_space_available(fd);
                }
            }

            /* If call was made on list head. */
            if (((FS *)fd)->flags & FS_CHAIN_HEAD)
            {
                /* We should not be in a list. */
                OS_ASSERT(is_list == TRUE);

                /* We are in a list. */
                is_list = TRUE;

                /* Pick-up the list head. */
                next_fd = ((FS *)fd)->fd_chain.fd_list.head;
            }

            else if (is_list == TRUE)
            {
                /* Pick-up the next file descriptor. */
                next_fd = ((FS *)fd)->fd_chain.fd_node.next;
            }

            if (((FS *)fd)->release_lock)
            {
                /* Release lock for this file descriptor. */
                ((FS *)fd)->release_lock((void *)fd);
            }
        }
        else
        {
            /* Unable to obtain semaphore. */
            break;
        }

        /* Check if we need to process a file descriptor in the chain. */
        if (next_fd != NULL)
        {
            /* Pick the next file descriptor. */
            fd = next_fd;

            /* Increment number of file descriptor processed. */
            n_fd++;
        }

    } while (next_fd != NULL);

    /* If we have written on more than one file descriptor. */
    if (n_fd > 1)
    {
        /* Calculate average number of bytes written. */
        written /= n_fd;
    }

    /* Return total number of bytes written. */
    return (written);

} /* fs_write */

/*
 * fs_ioctl
 * @fd: File descriptor.
 * @cmd: IOCTL command needed to be executed.
 * @param: IOCTL command parameter if any.
 * This function will execute a command on a file descriptor.
 */
int32_t fs_ioctl(FD fd, uint32_t cmd, void *param)
{
    int32_t status = SUCCESS;

    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        status = ((FS *)fd)->get_lock((void *)fd);
    }

    /* If lock was successfully obtained. */
    if (status == SUCCESS)
    {
        /* Check if an IOCTL function was registered with this descriptor. */
        if (((FS *)fd)->ioctl != NULL)
        {
            /* Transfer call to underlying API. */
            status = ((FS *)fd)->ioctl((void *)fd, cmd, param);
        }

        if (((FS *)fd)->release_lock)
        {
            /* Release lock for this file descriptor. */
            ((FS *)fd)->release_lock((void *)fd);
        }
    }

    /* Return status to caller. */
    return (status);

} /* fs_ioctl */

/*
 * fd_sreach_task
 * @node: An task waiting on this file system.
 * @param: Resumption criteria.
 * @return: TRUE if we need to resume this task, FALSE if we cannot resume
 *  this task.
 * This is a search function to search a task that satisfy the suspension
 * criteria.
 */
static uint8_t fd_sreach_task(void *node, void *param)
{
    FS_PARAM *fs_param = (FS_PARAM *)param;
    uint8_t match = FALSE;

    /* Check if the waiting task fulfills our criteria. */
    if (fs_param->flag & ((FS_PARAM *)((TASK *)node)->suspend_data)->flag)
    {
        /* Got an match. */
        match = TRUE;
    }

    /* Return if we need to stop the search or need to process more. */
    return (match);

} /* fs_sreach_task */

/*
 * fd_suspend_criteria
 * @fd: File descriptor on which task is needed to be suspended to wait for a
 *  criteria.
 * @param: Suspend criteria.
 * @timeout: Number of ticks to wait, or use MAX_WAIT to wait indefinitely.
 * @return: SUCCESS if criteria was successfully achieved, FS_TIMEOUT will be
 *  returned if a timeout was occurred while waiting for the giver criteria,
 *  or a status will be returned as returned in the task.
 * This function will suspend the caller task to wait for a criteria. Caller
 * must have acquired the FD lock before calling this routine, if required
 * lock will be released, if successful will be again acquired.
 */
int32_t fd_suspend_criteria(void *fd, FS_PARAM *param, uint32_t timeout)
{
    TASK *tcb = get_current_task();
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();
    int32_t status = SUCCESS;

    /* Current task should not be null. */
    OS_ASSERT(tcb == NULL);

#ifdef CONFIG_SLEEP
    /* Check if we need to wait for a finite time. */
    if (timeout != (uint32_t)(MAX_WAIT))
    {
        /* Add the current task to the sleep list, if not available in
         * the allowed time the task will be resumed. */
        sleep_add_to_list(tcb, ((FS *)fd)->timeout - current_system_tick());
    }
#endif /* CONFIG_SLEEP */

    /* If we need to sort the list on priority. */
    if (((FS *)fd)->flags & FS_PRIORITY_SORT)
    {
        /* Add this task on the file descriptor task list. */
        sll_insert(&((FS *)fd)->task_list, tcb, &task_priority_sort, OFFSETOF(TASK, next));
    }

    else
    {
        /* Add this task at the end of task list. */
        sll_append(&((FS *)fd)->task_list, tcb, OFFSETOF(TASK, next));
    }

    /* We need to suspend so disable preemption and release the lock
     * this way releasing the lock will not pass the control to
     * next task. */

    /* Disable preemption. */
    scheduler_lock();

    /* Disable interrupts. */
    /* File system semaphores can be accessed through the interrupts for
     * data protection, so disable the interrupts before releasing the
     * semaphore as this will cause task lists to get corrupted if this task
     * is needed to be resumed again from an interrupt. */
    DISABLE_INTERRUPTS();

    if (((FS *)fd)->release_lock)
    {
        /* Release lock for this file descriptor. */
        ((FS *)fd)->release_lock((void *)fd);
    }

    /* Task is being suspended. */
    tcb->status = TASK_SUSPENDED;

    /* Assign the suspension data to the task. */
    tcb->suspend_data = (void *)param;

    /* Wait for either being resumed by some data or timeout. */
    task_waiting();

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Check if we are resumed due to a timeout. */
    if (tcb->status == TASK_RESUME_SLEEP)
    {
        /* Return an error that we were failed to receive data
         * before given timeout. */
        status = FS_TIMEOUT;

        /* Remove this task from the file descriptor task list. */
        OS_ASSERT(sll_remove(&((FS *)fd)->task_list, tcb, OFFSETOF(TASK, next)) != tcb);
    }

    else if (tcb->status != TASK_RESUME)
    {
        /* Return the error returned by the task. */
        status = tcb->status;
    }

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

    /* Enable preemption. */
    scheduler_unlock();

    /* If the file node has been deleted then we should not try to get the
     * lock. */
    if (status != FS_NODE_DELETED)
    {
        /* Check if we need to get the lock. */
        if (((FS *)fd)->get_lock)
        {
            /* Get lock for this file descriptor. */
            OS_ASSERT(((FS *)fd)->get_lock((void *)fd) != SUCCESS);
        }
    }

    /* Return status to the caller. */
    return (status);

} /* fd_suspend_criteria */

/*
 * fd_data_available
 * @fd: File descriptor on which some data is available to read.
 * This function will be called be underlying file system to tell that there
 * is some data available to be read on this file descriptor.
 */
void fd_data_available(void *fd)
{
    FS_PARAM fs_param;
    FS_DATA_WATCHER *watcher = ((FS *)fd)->data_watcher_list.head;

    /* Set flag that some data is available. */
    ((FS *)fd)->flags |= FS_DATA_AVAILABLE;

    /* Call the consumer, this can be called from an interrupt so locks
     * must not be used here, also if called from user space appropriate
     * locks are already acquired. */

    /* While we have a watcher to process. */
    while ( (watcher != NULL) &&

            /* While we still have some data. */
            (((FS *)fd)->flags & FS_DATA_AVAILABLE))
    {
        /* If we have a watcher for received data. */
        if (watcher->data_available != NULL)
        {
            /* Call the watcher function. */
            watcher->data_available(fd, watcher->data);
        }

        /* Pick the next watcher. */
        watcher = watcher->next;
    }

    /* If we still have some data available, resume any tasks waiting on it. */
    if (((FS *)fd)->flags & FS_DATA_AVAILABLE)
    {
        /* Initialize criteria. */
        fs_param.flag = FS_BLOCK_READ;

        /* Resume a task if any waiting on read on this file descriptor. */
        fd_handle_criteria(fd, &fs_param);
    }

} /* fd_data_available */

/*
 * fd_data_flushed
 * @fs: File descriptor for which data has been flushed.
 * This function will clear the data available flag for a given file
 * descriptor. Caller must have the lock for FS before calling this routine.
 */
void fd_data_flushed(void *fd)
{
    /* Clear the data available flag. */
    ((FS *)fd)->flags &= (uint32_t)~(FS_DATA_AVAILABLE);

} /* fd_data_flushed */

/*
 * fd_space_available
 * @fs: File descriptor for which there is data space is available.
 * This function will clear the FD flag that is there is some space available
 * that can now be used, and resume any tasks waiting on this.
 */
void fd_space_available(void *fd)
{
    FS_PARAM fs_param;
    FS_DATA_WATCHER *watcher = ((FS *)fd)->data_watcher_list.head;

    /* Set flag that some data is available. */
    ((FS *)fd)->flags |= FS_SPACE_AVAILABLE;

    /* Call the consumer, this can be called from an interrupt so locks
     * must not be used here, also if called from user space appropriate
     * locks are already acquired. */

    /* While we have a watcher to process. */
    while ( (watcher != NULL) &&

            /* While we still have some data. */
            (((FS *)fd)->flags & FS_SPACE_AVAILABLE))
    {
        /* If we have somebody waiting for space on this file descriptor. */
        if (watcher->space_available != NULL)
        {
            /* Call the watcher function. */
            watcher->space_available(fd, watcher->data);
        }

        /* Pick the next watcher. */
        watcher = watcher->next;
    }

    /* If there is still some space available. */
    if (((FS *)fd)->flags & FS_SPACE_AVAILABLE)
    {
        /* Initialize criteria. */
        fs_param.flag = FS_BLOCK_WRITE;

        /* Resume a task if any waiting on write for this file descriptor. */
        fd_handle_criteria(fd, &fs_param);
    }

} /* fd_space_available */

/*
 * fd_space_consumed
 * @fs: File descriptor for which there is no more space.
 * This function will set the no space available flag for a given file
 * descriptor. Caller must have the lock for FS before calling this routine.
 */
void fd_space_consumed(void *fd)
{
    /* Set the flag that there is no more space in this FD. */
    ((FS *)fd)->flags &= (uint32_t)(~FS_SPACE_AVAILABLE);

} /* fd_space_consumed */

/*
 * fd_handle_criteria
 * @fd: File descriptor for which a criteria is needed to be handled.
 * @param: Criteria needed to be handled.
 * This function will update and handle criteria for an FD, also resumes a
 * task waiting for given criteria.
 */
void fd_handle_criteria(void *fd, FS_PARAM *param)
{
    /* Resume first task waiting on this file descriptor. */
    fs_resume_tasks(fd, TASK_RESUME, param, 1);

} /* fd_handle_criteria */

/*
 * fs_resume_tasks
 * @fs: File system for which waiting task(s) is needed to be resumed.
 * @status: Status to be returned.
 * @param: Search parameter to be used.
 * @n: Number of tasks to be resumed from the head.
 * This function will resume n tasks waiting on a FS.
 */
void fs_resume_tasks(void *fd, int32_t status, FS_PARAM *param, uint32_t n)
{
    TASK *tcb;

    /* Resume n tasks in this list. */
    while (n > 0)
    {
        /* If a parameter was given. */
        if (param != NULL)
        {

            /* Search for a task that can be resumed. */
            tcb = (TASK *)sll_search_pop(&((FS *)fd)->task_list, fd_sreach_task, param, OFFSETOF(TASK, next));
        }

        else
        {
            /* Get the first task that can be executed. */
            tcb = (TASK *)sll_pop(&((FS *)fd)->task_list, OFFSETOF(TASK, next));
        }

        /* If we have a task to resume. */
        if (tcb)
        {
            /* Task is resuming because of an error. */
            tcb->status = status;

#ifdef CONFIG_SLEEP
            /* Remove this task from sleeping tasks. */
            sleep_remove_from_list(tcb);
#endif /* CONFIG_SLEEP */

            /* Try to reschedule this task. */
            ((SCHEDULER *)(tcb->scheduler))->yield(tcb, YIELD_SYSTEM);

            /* Try to yield the current task. */
            task_yield();

            /* A task has been processed. */
            n--;
        }
        else
        {
            /* No more tasks left break out of this loop. */
            break;
        }
    }

} /* fs_resume_tasks */

#endif /* CONFIG_FS */
