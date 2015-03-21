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
 * fs_buffer_init
 * @buffer: File buffer needed to be initialized.
 * @data: Data space needed to be used for this buffer.
 * @size: Size of the data allocated for this buffer.
 * This function will initialize a buffer with given data.
 */
void fs_buffer_init(FS_BUFFER *buffer, char *data, uint32_t size)
{
    /* Clear this buffer. */
    memset(buffer, 0, sizeof(FS_BUFFER));

    /* Initialize this buffer. */
    buffer->data = buffer->buffer = data;
    buffer->max_length = size;

} /* fs_buffer_init */

/*
 * fs_buffer_add_head
 * @buffer: File buffer needed to be updated.
 * @size: Size of head room needed to be left in the buffer.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will add a head room to the given buffer, if there is already
 * some data on the buffer it will be moved. If it already has some head room
 * it will be maintained.
 */
int32_t fs_buffer_add_head(FS_BUFFER *buffer, uint32_t size)
{
    int32_t status = SUCCESS;

    /* An empty buffer should not come here. */
    OS_ASSERT(buffer->data == NULL);

    /* Validate that there is enough space on the buffer. */
    if (FS_BUFFER_SPACE(buffer) >= size)
    {
        /* Check if we have some data on the buffer. */
        if (buffer->length != 0)
        {
            /* Move the data to make room for head. */
            memmove(&buffer->buffer[size], buffer->buffer, buffer->length);
        }

        /* Update the buffer pointer. */
        buffer->buffer = buffer->buffer + size;
    }
    else
    {
        /* Return an error. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_add_head */

/*
 * fs_buffer_update
 * @buffer: File buffer needed to be updated.
 * @data: New buffer pointer.
 * @size: Size of valid data in the buffer.
 * This function will update a buffer data pointers.
 */
void fs_buffer_update(FS_BUFFER *buffer, char *data, uint32_t size)
{
    /* Update the buffer data. */
    buffer->buffer = data;
    buffer->length = size;

} /* fs_buffer_update */

/*
 * fs_buffer_chain_append
 * @chain: Buffer chain to which a buffer is needed to be added.
 * @buffer: Buffer needed to be added.
 * @flags: Defines where the given buffer is needed to be added.
 *  FS_BUFFER_HEAD: If we need to add this buffer on the chain head.
 * This function will add a given to a given buffer chain.
 */
void fs_buffer_chain_append(FS_BUFFER_CHAIN *chain, FS_BUFFER *buffer, uint8_t flag)
{
    /* If we need to add this buffer on the head. */
    if (flag & FS_BUFFER_HEAD)
    {
        /* Add this buffer on the head of the list. */
        sll_push(chain, buffer, OFFSETOF(FS_BUFFER, next));
    }

    else
    {
        /* Add this buffer at the end of the list. */
        sll_append(chain, buffer, OFFSETOF(FS_BUFFER, next));
    }

    /* Update the total length of this buffer chain. */
    chain->total_length += buffer->length;

} /* fs_buffer_chain_append */

/*
 * fs_buffer_one_pull
 * @buffer: File buffer from which data is needed to be pulled.
 * @data: Buffer in which data is needed to be pulled.
 * @size: Number of bytes needed to be pulled.
 * @flags: Defines how we will be pulling the data.
 *  FS_BUFFER_MSB_FIRST: If we need to pull the last byte first.
 *  FS_BUFFER_TAIL: If we need to pull data from the tail.
 *  FS_BUFFER_INPLACE: If we are just peeking and don't want data to be removed
 *      actually.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will remove data from a given buffer. If given will also copy
 * the data in the provided buffer.
 */
int32_t fs_buffer_one_pull(FS_BUFFER *buffer, char *data, uint32_t size, uint8_t flags)
{
    char *from;
    int32_t status = SUCCESS;

    /* Validate if we do have that much data on the buffer. */
    if (buffer->length >= size)
    {
        /* If we need to pull data from the tail. */
        if (flags & FS_BUFFER_TAIL)
        {
            /* Pick the data from the end of the buffer. */
            from = &buffer->buffer[buffer->length - size];
        }
        else
        {
            /* Pick the data from the start of the buffer. */
            from = buffer->buffer;

            /* If we don't want data to be removed. */
            if ((flags & FS_BUFFER_INPLACE) == 0)
            {
                /* Advance the buffer pointer. */
                buffer->buffer += size;
            }
        }

        /* If we need to actually need to return the pulled data. */
        if (data != NULL)
        {
            if (flags & FS_BUFFER_MSB_FIRST)
            {
                /* Copy the last byte first. */
                fs_memcpy_r(data, from, size);
            }
            else
            {
                /* Copy the data in the provided buffer. */
                memcpy(data, from, size);
            }
        }

        /* If we don't want data to be removed. */
        if ((flags & FS_BUFFER_INPLACE) == 0)
        {
            /* Update the buffer length. */
            buffer->length -= size;
        }
    }
    else
    {
        /* Return an error. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_one_pull */

/*
 * fs_buffer_chain_pull
 * @buffer_chain: File buffer chain from which data is needed to be pulled.
 * @data: Buffer in which data is needed to be pulled.
 * @size: Number of bytes needed to be pulled.
 * @flags: Defines how we will be pulling the data.
 *  FS_BUFFER_MSB_FIRST: If we need to pull the last byte first.
 *  FS_BUFFER_TAIL: If we need to pull data from the tail.
 *  FS_BUFFER_INPLACE: If we are just peeking and don't want data to be removed
 *      actually.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will remove data from a buffer chain. If given will also copy
 * the data in the provided buffer.
 */
int32_t fs_buffer_chain_pull(FS_BUFFER_CHAIN *buffer_chain, char *data, uint32_t size, uint8_t flags)
{
    FS_BUFFER *buffer;
    int32_t status = SUCCESS;
    uint32_t this_size;
    uint8_t reverse = ((flags & FS_BUFFER_MSB_FIRST) != 0);
    char *to;

    /* Validate if we do have that much data on the buffer chain. */
    if (buffer_chain->total_length >= size)
    {
        /* While we have some data to copy. */
        while (size > 0)
        {
            /* If we need to pull data from the tail. */
            if (flags & FS_BUFFER_TAIL)
            {
                /* Pick the tail buffer. */
                buffer = buffer_chain->list.tail;
            }
            else
            {
                /* Pick the head buffer. */
                buffer = buffer_chain->list.head;
            }

            /* There is data in the buffer so there must a buffer on the
             * chain. */
            OS_ASSERT(buffer == NULL);

            /* There should not be a zero buffer in the chain. */
            OS_ASSERT(buffer->length == 0);

            /* Check if we need to do a partial read of this buffer. */
            if (size > buffer->length)
            {
                /* Only copy the number of bytes that can be copied from
                 * this buffer. */
                this_size = buffer->length;
            }
            else
            {
                /* Copy the given number of bytes. */
                this_size = size;
            }

            /* Pick the destination pointer. */
            to = data;

            /* If we need to copy data MSB first. */
            if ((data != NULL) && (reverse == TRUE))
            {
                /* Move to the offset at the end of the provided buffer. */
                to += (size - this_size);
            }

            /* Pull data from this buffer. */
            /* We have already verified that we have enough length on the buffer
             * chain, so we should never get an error here. */
            OS_ASSERT(fs_buffer_one_pull(buffer, data, this_size, flags) != SUCCESS);

            /* If there is no more valid data on this buffer. */
            if (buffer->length == 0)
            {
                /* We no longer need this buffer on our buffer chain. */
                OS_ASSERT(sll_remove(&buffer_chain->list, buffer, OFFSETOF(FS_BUFFER, next)) != buffer);

                /* Actively free this buffer. */
                fs_buffer_add(buffer_chain->fd, buffer, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);
            }

            /* Decrement the number of bytes we still need to copy. */
            size = (uint32_t)(size - this_size);

            /* If we are returning the data. */
            if ((data != NULL) && (reverse == FALSE))
            {
                /* Update the data pointer. */
                data += this_size;
            }

            /* If we are not peeking the data. */
            if ((flags & FS_BUFFER_INPLACE) == 0)
            {
                /* Decrement number of bytes we have left on this buffer
                 * chain. */
                buffer_chain->total_length = (buffer_chain->total_length - this_size);
            }
        }
    }
    else
    {
        /* Return an error. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_chain_pull */

/*
 * fs_buffer_one_push
 * @buffer: File buffer on which data is needed to be pushed.
 * @data: Buffer from which data is needed to pushed.
 * @size: Number of bytes needed to be pushed.
 * @flags: Defines how we will be pushing the data.
 *  FS_BUFFER_MSB_FIRST: If we need to push the last byte first.
 *  FS_BUFFER_HEAD: If data is needed to be pushed on the head.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  buffer.
 * This function will add data in the buffer.
 */
int32_t fs_buffer_one_push(FS_BUFFER *buffer, char *data, uint32_t size, uint8_t flags)
{
    int32_t status = SUCCESS;
    char *to;

    /* An empty buffer should not come here. */
    OS_ASSERT(buffer->data == NULL);

    /* If we do have enough space on the buffer. */
    if ( ((flags & FS_BUFFER_HEAD) && (FS_BUFFER_SPACE(buffer) >= size)) ||
         (((flags & FS_BUFFER_HEAD) == 0) && (FS_BUFFER_TAIL_ROOM(buffer) >= size)) )
    {
        /* If we need to push data on the head. */
        if (flags & FS_BUFFER_HEAD)
        {
            /* Check if we have don't have enough head room. */
            if (FS_BUFFER_HEAD_ROOM(buffer) < size)
            {
                /* Add required head room. */
                status = fs_buffer_add_head(buffer, (size - FS_BUFFER_HEAD_ROOM(buffer)));
            }

            if (status == SUCCESS)
            {
                /* We will be adding data at the start of the existing data. */

                /* Decrement the buffer pointer. */
                buffer->buffer -= size;

                /* Pick the pointer at which we will be adding data. */
                to = buffer->buffer;
            }
        }
        else
        {
            /* We will be pushing data at the end of the buffer. */
            to = &buffer->buffer[buffer->length];
        }

        if (status == SUCCESS)
        {
            /* If we actually need to push some data. */
            if (data != NULL)
            {
                if (flags & FS_BUFFER_MSB_FIRST)
                {
                    /* Copy data from the provided buffer last byte first. */
                    fs_memcpy_r(to, data, size);
                }

                else
                {
                    /* Copy data from the provided buffer. */
                    memcpy(to, data, size);
                }
            }

            /* Update the buffer length. */
            buffer->length += size;
        }
    }
    else
    {
        /* Return an error. */
        status = FS_BUFFER_NO_SPACE;
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_one_push */

/*
 * fs_buffer_chain_push
 * @buffer_chain: File buffer chain on which data is needed to be pushed.
 * @data: Buffer from which data is needed to pushed.
 * @size: Number of bytes needed to be pushed.
 * @flags: Defines how we will be pushing the data.
 *  FS_BUFFER_MSB_FIRST: If we need to push the last byte first.
 *  FS_BUFFER_HEAD: If data is needed to be pushed on the head.
 * @return: Success if operation was successfully performed,
 *  FS_BUFFER_NO_SPACE will be returned if there is not enough space in the
 *  file descriptor for new buffers.
 * This function will add data in the buffer chain.
 */
int32_t fs_buffer_chain_push(FS_BUFFER_CHAIN *buffer_chain, char *data, uint32_t size, uint8_t flags)
{
    int32_t status = SUCCESS;
    FS_BUFFER *buffer;
    uint32_t this_size;
    uint8_t reverse = ((flags & FS_BUFFER_MSB_FIRST) != 0);
    char *from;

    /* While we have data to copy. */
    while ((status == SUCCESS) && (size > 0))
    {
        /* If we need to push data on the head. */
        if (flags & FS_BUFFER_HEAD)
        {
            /* Pick the head buffer. */
            buffer = buffer_chain->list.head;

            /* Either we don't have a buffer in the chain or there is no space
             * on the head buffer. */
            if ((buffer == NULL) || (FS_BUFFER_SPACE(buffer) == 0))
            {
                /* Need to allocate a new buffer to be pushed on the head. */
                buffer = fs_buffer_get(buffer_chain->fd, FS_BUFFER_FREE, 0);

                /* If a buffer was allocated. */
                if (buffer)
                {
                    /* Use all the space in this buffer as head room. */
                    OS_ASSERT(fs_buffer_add_head(buffer, buffer->length) != SUCCESS);

                    /* Add this buffer on the head of the buffer chain. */
                    sll_push(buffer_chain, buffer, OFFSETOF(FS_BUFFER, next));
                }
            }

            if (buffer != NULL)
            {
                /* Pick the number of bytes we can copy on this buffer. */
                this_size = FS_BUFFER_SPACE(buffer);

                /* If we have more space then we require. */
                if (this_size > size)
                {
                    /* Copy the required amount of data. */
                    this_size = size;
                }
            }
            else
            {
                /* There is no space on the file descriptor. */
                status = FS_BUFFER_NO_SPACE;
            }
        }
        else
        {
            /* Pick the tail buffer. */
            buffer = buffer_chain->list.tail;

            /* Either we don't have a buffer in the chain or there is no space
             * on the tail buffer. */
            if ((buffer == NULL) || (FS_BUFFER_TAIL_ROOM(buffer) == 0))
            {
                /* Need to allocate a new buffer to be appended on the tail. */
                buffer = fs_buffer_get(buffer_chain->fd, FS_BUFFER_FREE, 0);

                /* If a buffer was allocated. */
                if (buffer)
                {
                    /* Append this buffer at the end of buffer chain. */
                    sll_append(buffer_chain, buffer, OFFSETOF(FS_BUFFER, next));
                }
            }

            if (buffer != NULL)
            {
                /* Pick the number of bytes we can copy on this buffer. */
                this_size = FS_BUFFER_TAIL_ROOM(buffer);

                /* If we have more space then we require. */
                if (this_size > size)
                {
                    /* Copy the required amount of data. */
                    this_size = size;
                }
            }
            else
            {
                /* There is no space on the file descriptor. */
                status = FS_BUFFER_NO_SPACE;
            }
        }

        if (status == SUCCESS)
        {
            /* Pick the source pointer. */
            from = data;

            /* If we need to copy data MSB first. */
            if ((data != NULL) && (reverse == TRUE))
            {
                /* Move to the offset at the end of the provided buffer. */
                from += (size - this_size);
            }

            /* Push data on the buffer we have selected. */
            OS_ASSERT(fs_buffer_one_push(buffer, from, this_size, flags) != SUCCESS);

            /* Update the buffer chain size. */
            buffer_chain->total_length += this_size;

            /* Decrement the bytes we have copied in this go. */
            size = size - this_size;

            /* If we are copying data normally. */
            if ((data != NULL) && (reverse == FALSE))
            {
                /* Update the data pointer. */
                data += this_size;
            }
        }
    }

    /* Return status to the caller. */
    return (status);

} /* fs_buffer_chain_push */

/*
 * fs_buffer_dataset
 * @fd: File descriptor for which buffer data-set is needed to be set.
 * @data: Pointer to buffer data structure.
 * @num_buffers: Total number of buffers in the system.
 * This function will set the buffer structure to be used by this file
 * descriptor also sets the flag to tell others that this will be a buffered
 * file descriptor.
 */
void fs_buffer_dataset(FD fd, FS_BUFFER_DATA *data, int32_t num_buffers)
{
    /* Should never happen. */
    OS_ASSERT(data == NULL);

    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        OS_ASSERT(((FS *)fd)->get_lock((void *)fd) != SUCCESS);
    }

    /* This is a buffer file system. */
    ((FS *)fd)->flags |= FS_BUFFERED;

    /* Set the buffer data structure provided by caller. */
    ((FS *)fd)->buffer = data;

    /* Clear the structure. */
    memset(data, 0, sizeof(FS_BUFFER_DATA));
    data->buffers = num_buffers;

    if (((FS *)fd)->release_lock)
    {
        /* Release lock for this file descriptor. */
        ((FS *)fd)->release_lock((void *)fd);
    }

} /* fs_buffer_dataset */

/*
 * fs_buffer_chain_add
 * @chain: Buffer chain needed to be added.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_FREE: If this is a free buffer.
 *  FS_BUFFER_RX: If this is a receive buffer.
 *  FS_BUFFER_TX: If this is a transmit buffer.
 * @flags: Operation flags.
 *  FS_BUFFER_ACTIVE: Actively add the buffer and invoke the any callbacks.
 * This function will add a buffer chain in the file descriptor for the required
 * type.
 */
void fs_buffer_chain_add(FS_BUFFER_CHAIN *chain, uint32_t type, uint32_t flags)
{
    FS_BUFFER *buffer;

    do
    {
        /* Pick a buffer from the buffer list. */
        buffer = sll_pop(chain, OFFSETOF(FS_BUFFER, next));

        if (buffer)
        {
            /* Add a buffer from the chain to the given file descriptor. */
            fs_buffer_add(chain->fd, buffer, type, flags);
        }

    } while (buffer != NULL);

    /* There are no more buffers, reset the chain length. */
    chain->total_length = 0;

} /* fs_buffer_chain_add */

/*
 * fs_buffer_add
 * @fd: File descriptor on which a free buffer is needed to be added.
 * @buffer: Buffer needed to be added.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_FREE: If this is a free buffer.
 *  FS_BUFFER_RX: If this is a receive buffer.
 *  FS_BUFFER_TX: If this is a transmit buffer.
 * @flags: Operation flags.
 *  FS_BUFFER_ACTIVE: Actively add the buffer and invoke the any callbacks.
 * This function will add a new buffer in the file descriptor for the required
 * type.
 */
void fs_buffer_add(FD fd, FS_BUFFER *buffer, uint32_t type, uint32_t flags)
{
    FS_BUFFER_DATA *data = ((FS *)fd)->buffer;

    /* Should never happen. */
    OS_ASSERT(data == NULL);

    /* Type of buffer we are adding. */
    switch (type)
    {
    case (FS_BUFFER_FREE):
        /* Initialize this buffer. */
        buffer->buffer = buffer->data;
        buffer->length = 0;

        /* Just add this buffer in the free buffer list. */
        sll_append(&data->free_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));

#ifdef FS_BUFFER_TRACE
        /* Increment the number of buffers on free list. */
        data->free_buffer_list.buffers ++;
#endif

#ifdef FS_BUFFER_TRACE
        /* Should not happen. */
        OS_ASSERT(data->buffers == 0);

        /* Decrement number of buffers in the system. */
        data->buffers --;
#endif

        /* If we are doing this actively. */
        if (flags & FS_BUFFER_ACTIVE)
        {
            /* Some space is now available. */
            fd_space_available(fd);
        }
        else
        {
            /* Just set flag that some data is available. */
            ((FS *)fd)->flags |= FS_SPACE_AVAILABLE;
        }

        break;

    case (FS_BUFFER_RX):

        /* Just add this buffer in the receive buffer list. */
        sll_append(&data->rx_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));

#ifdef FS_BUFFER_TRACE
        /* Increment the number of buffers on receive list. */
        data->rx_buffer_list.buffers ++;
#endif

#ifdef FS_BUFFER_TRACE
        /* Should not happen. */
        OS_ASSERT(data->buffers == 0);

        /* Decrement number of buffers in the system. */
        data->buffers --;
#endif

        /* If we are doing this actively. */
        if (flags & FS_BUFFER_ACTIVE)
        {
            /* Some new data is now available. */
            fd_data_available(fd);
        }
        else
        {
            /* Just set the flags that data is available. */
            ((FS *)fd)->flags |= FS_DATA_AVAILABLE;
        }

        break;

    case (FS_BUFFER_TX):

        /* Just add this buffer in the transmit buffer list. */
        sll_append(&data->tx_buffer_list, buffer, OFFSETOF(FS_BUFFER, next));

#ifdef FS_BUFFER_TRACE
        /* Increment the number of buffers on transmit list. */
        data->tx_buffer_list.buffers ++;
#endif

#ifdef FS_BUFFER_TRACE
        /* Should not happen. */
        OS_ASSERT(data->buffers == 0);

        /* Decrement number of buffers in the system. */
        data->buffers --;
#endif

        break;
    }

} /* fs_buffer_add */

/*
 * fs_buffer_get
 * @fd: File descriptor from which a free buffer is needed.
 * @type: Type of buffer needed to be added.
 *  FS_BUFFER_FREE: If this is a free buffer.
 *  FS_BUFFER_RX: If this is a receive buffer.
 *  FS_BUFFER_TX: If this is a transmit buffer.
 * @flags: Operation flags.
 *  FS_BUFFER_INPLACE: Will not remove the buffer from the list just return a
 *      pointer to it.
 * This function return a buffer from a required buffer list for this file
 * descriptor.
 */
FS_BUFFER *fs_buffer_get(FD fd, uint32_t type, uint32_t flags)
{
    FS_BUFFER_DATA *data = ((FS *)fd)->buffer;
    FS_BUFFER *buffer = NULL;

    /* Should never happen. */
    OS_ASSERT(data == NULL);

    /* Type of buffer we need. */
    switch (type)
    {
    case (FS_BUFFER_FREE):

        /* If we need to return the buffer in-place. */
        if (flags & FS_BUFFER_INPLACE)
        {
            /* Return the pointer to the head buffer. */
            buffer = data->free_buffer_list.head;
        }
        else
        {
            /* Pop a buffer from this file descriptor's free buffer list. */
            buffer = sll_pop(&data->free_buffer_list, OFFSETOF(FS_BUFFER, next));

#ifdef FS_BUFFER_TRACE
            /* If we are returning a buffer. */
            if (buffer)
            {
                /* Decrement the number of buffers on free list. */
                data->free_buffer_list.buffers --;
            }
#endif

            /* If we don't have any more free space on this file descriptor. */
            if (data->free_buffer_list.head == NULL)
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
            buffer = data->rx_buffer_list.head;
        }
        else
        {
            /* Pop a buffer from this file descriptor's receive buffer list. */
            buffer = sll_pop(&data->rx_buffer_list, OFFSETOF(FS_BUFFER, next));

#ifdef FS_BUFFER_TRACE
            /* If we are returning a buffer. */
            if (buffer)
            {
                /* Decrement the number of buffers on receive list. */
                data->rx_buffer_list.buffers --;
            }
#endif

            /* If we don't have any more data to read. */
            if (data->rx_buffer_list.head == NULL)
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
            buffer = data->tx_buffer_list.head;
        }
        else
        {
            /* Pop a buffer from this file descriptor's transmit buffer list. */
            buffer = sll_pop(&data->tx_buffer_list, OFFSETOF(FS_BUFFER, next));

#ifdef FS_BUFFER_TRACE
            /* If we are returning a buffer. */
            if (buffer)
            {
                /* Decrement the number of buffers on transmit list. */
                data->tx_buffer_list.buffers --;
            }
#endif
        }

        break;
    }

    /* If we are returning a buffer. */
    if ( (buffer) &&
         ((flags & FS_BUFFER_INPLACE) == 0) )
    {
        /* Clear the next buffer pointer. */
        buffer->next = NULL;

#ifdef FS_BUFFER_TRACE
        /* Increase the number of buffers in the system. */
        data->buffers ++;
#endif
    }

    /* Return the buffer. */
    return (buffer);

} /* fs_buffer_get */

/*
 * fs_buffer_divide
 * @fd: File descriptor from which given buffer allocated.
 * @buffer: Buffer for which data is needed to be divided.
 * @new_buffer: Buffer that will have the remaining data of this buffer.
 * @data_ptr: Pointer in the original buffer at which this buffer is needed to
 *  be divided.
 * @data_len: Number of bytes valid in the second buffer.
 * This function will divide the given buffer into two buffers. An empty buffer
 * will allocated to hold the remaining portion of buffer.
 */
void fs_buffer_divide(FD fd, FS_BUFFER *buffer, FS_BUFFER **new_buffer,
                      char *data_ptr, uint32_t data_len)
{
    FS_BUFFER *ret_buffer = NULL;

    /* Divide the original buffer. */
    buffer->length -= data_len;

    /* Check if we really do need return the remaining buffer. */
    if (new_buffer != NULL)
    {
        /* Allocate a free buffer. */
        ret_buffer = fs_buffer_get(fd, FS_BUFFER_FREE, 0);

        /* If a free buffer was allocated. */
        if (ret_buffer != NULL)
        {
            /* Check if the new buffer has enough space to copy the data. */
            OS_ASSERT(ret_buffer->max_length < data_len);

            /* Copy data from old buffer to the new buffer. */
            memcpy(ret_buffer->buffer, data_ptr, data_len);
            ret_buffer->length = data_len;
        }

        /* Return the new buffer. */
        *new_buffer = ret_buffer;
    }

} /* fs_buffer_divide */

/*
 * fs_data_watcher_set
 * @fd: File descriptor for which data is needed to be monitored.
 * @watcher: Data watcher needed to be registered.
 * This function will add a data watcher for the given file system.
 */
void fs_data_watcher_set(FD fd, FS_DATA_WATCHER *watcher)
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

} /* fs_data_watcher_set */

/*
 * fs_connection_watcher_set
 * @fd: File descriptor for which connection is needed to be monitored.
 * @watcher: Connection watcher needed to be registered.
 * This function will add a connection watcher for given file system.
 */
void fs_connection_watcher_set(FD fd, FS_CONNECTION_WATCHER *watcher)
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

} /* fs_connection_watcher_set */

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

/*
 * fs_memcpy_r
 * @dst: Destination buffer on which data is needed to be pushed.
 * @src: Source buffer from which data is needed to be copied.
 * @len: Number of bytes to copy.
 * This function will copy n bytes from source to the destination buffer last
 * byte first.
 */
void fs_memcpy_r(char *dst, char *src, uint32_t n)
{
    /* Go to the end of destination buffer. */
    dst = dst + n;

    /* While we have a byte to copy. */
    while (n --)
    {
        /* Copy a byte from source to the buffer. */
        *(--dst) = *(src++);
    }

} /* fs_memcpy_r */

#endif /* CONFIG_FS */
