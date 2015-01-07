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

#ifdef CONFIG_FS

/* Global variables. */
FS_DATA file_data;

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
    semaphore_obtain(&file_data.lock, MAX_WAIT);
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
    uint32_t match = FALSE;

    /* Match the file system path. */
    if ( util_path_match(((FS *)node)->name, &path) == TRUE)
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
    uint32_t match = FALSE;

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

    /* Initialize a search parameter. */
    param.name = name;
    param.matched = NULL;
    param.priv = (void *)fd;

#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data lock. */
    semaphore_obtain(&file_data.lock, MAX_WAIT);
#endif

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
    if (((FS *)fd)->close != NULL)
    {
        /* Transfer call to underlying API. */
        ((FS *)fd)->close((void **)fd);
    }

    else
    {
        /* Clear the file descriptor. */
        *fd = (FD)NULL;
    }

} /* fs_close */

/*
 * fs_read
 * @fd: File descriptor.
 * @buffer: Data buffer.
 * @nbytes: Number of bytes to write.
 * This function will read data from a file descriptor.
 */
uint32_t fs_read(FD fd, char *buffer, uint32_t nbytes)
{
    uint32_t read = 0;

    /* Check if a read function was registered with this descriptor. */
    if (((FS *)fd)->read != NULL)
    {
        /* Transfer call to underlying API. */
        read = ((FS *)fd)->read((void *)fd, buffer, nbytes);
    }

    /* Return number of bytes read. */
    return (read);

} /* fs_read */

/*
 * fs_write
 * @fd: File descriptor.
 * @buffer: Data buffer.
 * @nbytes: Number of bytes to write.
 * This function will write data on a file descriptor.
 */
uint32_t fs_write(FD fd, char *buffer, uint32_t nbytes)
{
    uint32_t written = 0;

    /* Check if a write function was registered with this descriptor. */
    if (((FS *)fd)->write != NULL)
    {
        /* Transfer call to underlying API. */
        written = ((FS *)fd)->write((void *)fd, buffer, nbytes);
    }

    /* Return number of bytes written. */
    return (written);

} /* fs_write */

/*
 * fs_ioctl
 * @fd: File descriptor.
 * @cmd: IOCTL command needed to be executed.
 * @param: IOCTL command parameter if any.
 * This function will execute a command on a file descriptor.
 */
uint32_t fs_ioctl(FD fd, uint32_t cmd, void *param)
{
    uint32_t ret = 0;

    /* Check if an IOCTL function was registered with this descriptor. */
    if (((FS *)fd)->ioctl != NULL)
    {
        /* Transfer call to underlying API. */
        ret = ((FS *)fd)->ioctl((void *)fd, cmd, param);
    }

    return (ret);

} /* fs_ioctl */

#endif /* CONFIG_FS */
