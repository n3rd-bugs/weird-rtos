/*
 * pipe.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */

#include <kernel.h>
#include <fs.h>

#ifdef FS_PIPE
#include <string.h>
#include <sll.h>
#include <pipe.h>

/* Pipe FS data. */
static PIPE_DATA pipe_data;

/* Internal function prototypes. */
static int32_t pipe_lock(void *, uint32_t);
static void pipe_unlock(void *);
static void *pipe_open(void *, char *, uint32_t);

/* File system definition. */
FS pipe_fs =
{
        /* Pipe file system root node. */
        .name = "\\pipe",

        /* File manipulation API. */
        .open = pipe_open,
};

/*
 * pipe_init
 * This function will initialize pipe file system.
 */
void pipe_init()
{
    /* Clear the pipe data. */
    memset(&pipe_data, 0, sizeof(PIPE_DATA));

#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect global pipe data. */
    semaphore_create(&pipe_data.lock, 1, 1, FALSE);
#endif

    /* Register pipe with file system. */
    fs_register(&pipe_fs);

} /* pipe_init */

/*
 * pipe_create
 * @pipe: Pipe data to be registered.
 * @name: Name of the pipe being created.
 * @buffer: Buffer to be used to store the pipe data.
 * @size: Size of buffer.
 * This function will initialize and register a pipe.
 */
void pipe_create(PIPE *pipe, char *name, uint8_t *buffer, uint32_t size)
{
    NODE_PARAM param;

#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Obtain the global data lock. */
    ASSERT(semaphore_obtain(&pipe_data.lock, MAX_WAIT) != SUCCESS);
#endif

    /* First check if this node can be registered. */

    /* Initialize a search parameter. */
    param.name = name;
    param.priv = (void *)NULL;

    /* First check if these is no other pipe with same name. */
    sll_search(&pipe_data.list, NULL, &fs_sreach_node, &param, OFFSETOF(PIPE, fs.next));

    if (param.priv == NULL)
    {
        /* Clear pipe data. */
        memset(pipe, 0, sizeof(PIPE));

        /* Initialize PIPE data. */
        pipe->data = buffer;
        pipe->size = size;
        pipe->free = pipe->message = 0;
        ((MSG_DATA *)(pipe->data))->flags = 0;

        /* Initialize FS structure. */
        pipe->fs.name = name;
        pipe->fs.read = &pipe_read;
        pipe->fs.write = &pipe_write;
        pipe->fs.flags = (FS_BLOCK | FS_SPACE_AVAILABLE);
        pipe->fs.timeout = MAX_WAIT;
        pipe->fs.get_lock = pipe_lock;
        pipe->fs.release_lock = pipe_unlock;

        /* Initialize file system condition. */
        fs_condition_init(&pipe->fs);

#ifdef CONFIG_SEMAPHORE
        /* Create a semaphore to protect pipe data. */
        semaphore_create(&pipe->lock, 1, 1, FALSE);
#endif

        /* Just push this file system in the list. */
        sll_push(&pipe_data.list, pipe, OFFSETOF(PIPE, fs.next));
    }

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&pipe_data.lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif
} /* pipe_create */

/*
 * pipe_destroy
 * @pipe: Pipe data to be destroyed.
 * This function will destroy a pipe. User should not access a pipe
 * after destroying it.
 */
void pipe_destroy(PIPE *pipe)
{
    /* This could be a file descriptor chain, so destroy it. */
    fs_destroy_chain((FD)&pipe->fs);

#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else

    /* Obtain the global data lock. */
    ASSERT(semaphore_obtain(&pipe_data.lock, MAX_WAIT) != SUCCESS);

    /* Obtain data lock for this pipe. */
    if (semaphore_obtain(&pipe->lock, MAX_WAIT) == SUCCESS)
    {
#endif
        /* Resume all tasks waiting on this file descriptor. */
        fd_handle_criteria((FD)pipe, NULL, FS_NODE_DELETED);

#ifdef CONFIG_SEMAPHORE
        /* Delete the pipe semaphore. */
        semaphore_destroy(&pipe->lock);
#endif

        /* Just remove this pipe from the pipe list. */
        ASSERT(sll_remove(&pipe_data.list, pipe, OFFSETOF(PIPE, fs.next)) != pipe);

#ifdef CONFIG_SEMAPHORE
    }

    /* Release the global data lock. */
    semaphore_release(&pipe_data.lock);

#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif

} /* pipe_destroy */

/*
 * pipe_lock
 * @fd: File descriptor for the pipe.
 * @timeout: Number of ticks we need to wait for the lock.
 * This function will get the lock for a given pipe.
 */
static int32_t pipe_lock(void *fd, uint32_t timeout)
{
#ifdef CONFIG_SEMAPHORE
    /* Obtain data lock for this pipe. */
    return semaphore_obtain(&((PIPE *)fd)->lock, timeout);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(timeout);

    /* Lock scheduler. */
    scheduler_lock();

    /* Return success. */
    return (SUCCESS);
#endif
} /* pipe_lock */

/*
 * pipe_unlock
 * @fd: File descriptor for the pipe.
 * This function will release the lock for a given pipe.
 */
static void pipe_unlock(void *fd)
{
#ifdef CONFIG_SEMAPHORE
    /* Release data lock for this pipe. */
    semaphore_release(&((PIPE *)fd)->lock);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Enable scheduling. */
    scheduler_unlock();
#endif
} /* pipe_unlock */

/*
 * pipe_open
 * @priv_data: Private data.
 * @name: Pipe name.
 * @flags: Open flags.
 * This function will open a pipe node.
 */
static void *pipe_open(void *priv_data, char *name, uint32_t flags)
{
    NODE_PARAM param;
    void *fd = NULL;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(priv_data);
    UNUSED_PARAM(flags);

#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data lock. */
    ASSERT(semaphore_obtain(&pipe_data.lock, MAX_WAIT) != SUCCESS);
#endif

    /* Initialize a search parameter. */
    param.name = name;
    param.priv = (void *)fd;

    /* First find a file system to which this call can be forwarded. */
    sll_search(&pipe_data.list, NULL, &fs_sreach_node, &param, OFFSETOF(PIPE, fs.next));

    /* If a node was found. */
    if (param.priv)
    {
        /* Use this FD, we will update it if required. */
        fd = param.priv;
    }

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&pipe_data.lock);
#endif

    if (fd != NULL)
    {
        /* Check if we need to call the underlying function to get a new file
         * descriptor. */
        if (((PIPE *)fd)->fs.open != NULL)
        {
            /* Call the underlying API to get the file descriptor. */
            fd = ((PIPE *)fd)->fs.open(fd, name, flags);
        }
    }

    /* Return the file descriptor. */
    return (fd);

} /* pipe_open */

/*
 * pipe_write
 * @fd: File descriptor.
 * @data: Data to be written.
 * @nbytes: Number of bytes.
 * @return: Number of bytes written on this pipe.
 * This function will write data on pipe.
 */
int32_t pipe_write(void *fd, uint8_t *data, int32_t nbytes)
{
    PIPE *pipe = (PIPE *)fd;
    uint32_t required_space, part_size;
    MSG_DATA *message;

    /* Calculate required space. */
    required_space = ALLIGN_CEIL((uint32_t)nbytes) + sizeof(MSG_DATA);

    /* Get the current message. */
    message = (MSG_DATA *)(&pipe->data[pipe->message]);

    /* First check if we can push more data on pipe. */
    if ( (!(message->flags & PIPE_MSG_VALID)) ||
         (((pipe->free > pipe->message) &&
           ((pipe->free + required_space) < (pipe->size + pipe->message))) ||
          ((pipe->free < pipe->message) &&
           ((pipe->free + required_space) < pipe->message))) )
    {
        /* Push the message header. */
        message = (MSG_DATA *)(&pipe->data[pipe->free]);
        message->flags = 0;
        message->size = (uint32_t)nbytes;

        /* Move past the message header. */
        pipe->free += sizeof(MSG_DATA);

        /* After putting the header we could be at the end on the
         * assigned space. */
        if (pipe->free == pipe->size)
        {
            pipe->free = 0;
        }

        /* Check if we need to split the data. */
        if (((pipe->free + (uint32_t)nbytes) > pipe->size))
        {
            /* First part is always the data remaining in the pipe data buffer. */
            part_size = pipe->size - pipe->free;
        }
        else
        {
            /* No need to split just put all the data in one go. */
            part_size = (uint32_t)nbytes;
        }

        /* Now put the message data. */
        memcpy(&pipe->data[pipe->free], data, part_size);
        memcpy(pipe->data, (data + part_size), ((uint32_t)nbytes - part_size));

        /* Increment the free pointer. */
        pipe->free += ALLIGN_CEIL((uint32_t)nbytes);

        /* Check if we need to roll over the free. */
        if (pipe->free > pipe->size)
        {
            /* Roll over the current free. */
            pipe->free %= pipe->size;
        }

        /* There should be enough space to store a message header here. */
        else if (pipe->free + sizeof(MSG_DATA) > pipe->size)
        {
            /* Roll over the buffer. */
            pipe->free = 0;
        }

        /* Set the message flag as valid. */
        message->flags |= PIPE_MSG_VALID;

        /* Check if we cannot push a message on this PIPE. */
        if ( (message->flags & PIPE_MSG_VALID) &&
             (((pipe->free > pipe->message) &&
               ((pipe->free + sizeof(MSG_DATA)) >= (pipe->size + pipe->message))) ||
              ((pipe->free < pipe->message) &&
               ((pipe->free + sizeof(MSG_DATA)) >= pipe->message))) )
        {
            /* There is no more space on this PIPE. */
            fd_space_consumed(fd);
        }

        /* There is some data available to be read on this file descriptor. */
        fd_data_available(fd);
    }
    else
    {
        /* Nothing was written on the pipe. */
        nbytes = 0;
    }

    /* Return number of bytes. */
    return (nbytes);

} /* pipe_write */

/*
 * pipe_read
 * @fd: File descriptor.
 * @buffer: Buffer in which data will be read.
 * @size: Size of buffer.
 * @return: Number of bytes read from the pipe.
 * This function will read data from a pipe.
 */
int32_t pipe_read(void *fd, uint8_t *buffer, int32_t size)
{
    PIPE *pipe = (PIPE *)fd;
    MSG_DATA *message;
    uint32_t part_size, message_size;
    int32_t nbytes = 0;

    /* Get the current message. */
    message = (MSG_DATA *)(&pipe->data[pipe->message]);

    /* First check if we have a valid message on the queue. */
    if ((message->flags & PIPE_MSG_VALID) && ((uint32_t)size >= message->size))
    {
        /* Calculate message data length and actual message size. */
        nbytes = (int32_t)message->size;
        message_size = sizeof(MSG_DATA) + (uint32_t)nbytes;

        /* Check if we need to split the data. */
        if ((pipe->message + message_size) > pipe->size)
        {
            /* First part is always the data remaining in the pipe data buffer. */
            part_size = pipe->size - (pipe->message + sizeof(MSG_DATA));
        }
        else
        {
            /* No need to split just put all the data in one go. */
            part_size = (uint32_t)nbytes;
        }

        /* Copy data for the pipe in to the giver buffer. */
        memcpy(buffer, (char *)(message + 1), part_size);
        memcpy(buffer + part_size, pipe->data, (uint32_t)nbytes - part_size);

        /* Discard this message. */
        pipe->message += sizeof(MSG_DATA) + ALLIGN_CEIL(message->size);

        /* Check if we need to roll over the message. */
        if (pipe->message > pipe->size)
        {
            /* Roll over the current message. */
            pipe->message %= pipe->size;
        }

        /* There should be enough space to store a message header here. */
        else if (pipe->message + sizeof(MSG_DATA) > pipe->size)
        {
            /* Roll over the buffer. */
            pipe->message = 0;
        }

        /* If there are no more messages. */
        if (pipe->message == pipe->free)
        {
            /* Clear and reset this pipe. */
            pipe->message = pipe->free = 0;
            ((MSG_DATA *)(pipe->data))->flags = 0;

            /* Tell the file system that this pipe is now flushed */
            fd_data_flushed(fd);
        }
    }

    /* Return number of bytes. */
    return (nbytes);

} /* pipe_read */

#endif /* FS_PIPE */
