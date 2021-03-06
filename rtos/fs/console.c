/*
 * console.c
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
#include <fs.h>

#ifdef FS_CONSOLE
#include <string.h>
#include <path.h>
#include <sll.h>
#include <console.h>

/* Console data. */
static CONSOLE_DATA console_data;

/* Function prototypes. */
static void *console_open(void *, const char *, uint32_t);
static void console_unlock(void *);
static int32_t console_lock(void *, uint32_t);

/* File system definition. */
static FS console_fs =
{
        /* Console file system root node. */
        .name = "\\console",

        /* File manipulation API. */
        .open = console_open,

        /* Close, read, write and IOCTL will be populated by
         * the underlying device. */
};

/*
 * console_init
 * This function will initialize debug console.
 */
void console_init(void)
{
    /* Clear the console data. */
    memset(&console_data, 0, sizeof(CONSOLE_DATA));

#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect global console data. */
    semaphore_create(&console_data.lock, 1);
#endif /* CONFIG_SEMAPHORE */

    /* Register console with file system. */
    fs_register(&console_fs);

} /* console_init */

/*
 * console_register
 * @console: Console data.
 * This function will register a console.
 */
void console_register(CONSOLE *console)
{
#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Obtain the global data lock. */
    ASSERT(semaphore_obtain(&console_data.lock, MAX_WAIT) != SUCCESS);

    /* Create a semaphore to protect this console device. */
    memset(&console->lock, 0, sizeof(SEMAPHORE));
    semaphore_create(&console->lock, 1);
#endif

    /* This utility is called by drivers for registering consoles for the
     * applicable devices, so no need to check for name conflicts. */
    /* Just push this file system in the list. */
    sll_push(&console_data.list, console, OFFSETOF(CONSOLE, fs.next));

    /* Initialize console FS data. */
    console->fs.get_lock = console_lock;
    console->fs.release_lock = console_unlock;
#ifdef CONFIG_SLEEP
    console->fs.timeout = MAX_WAIT;
#endif /* CONFIG_SLEEP */

    /* Initialize file system condition. */
    fs_condition_init(&console->fs);

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&console_data.lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif /* CONFIG_SEMAPHORE */

} /* console_register */

/*
 * console_unregister
 * @console: Console data.
 * This function will unregister a console.
 */
void console_unregister(CONSOLE *console)
{
#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Obtain the global data lock. */
    ASSERT(semaphore_obtain(&console_data.lock, MAX_WAIT) != SUCCESS);

    /* Obtain the lock for the console needed to be unregistered. */
    if (semaphore_obtain(&console->lock, MAX_WAIT) == SUCCESS)
    {
#endif /* CONFIG_SEMAPHORE */
        /* Resume all tasks waiting on this file descriptor. */
        fd_handle_criteria((FD)console, NULL, FS_NODE_DELETED);

#ifdef CONFIG_SEMAPHORE
        /* Delete the console lock. */
        semaphore_destroy(&console->lock);
#endif /* CONFIG_SEMAPHORE */

        /* Just remove this console from console list. */
        ASSERT(sll_remove(&console_data.list, console, OFFSETOF(CONSOLE, fs.next)) != console);

#ifdef CONFIG_SEMAPHORE
    }

    /* Release the global data lock. */
    semaphore_release(&console_data.lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif /* CONFIG_SEMAPHORE */

} /* console_unregister */

/*
 * console_open
 * @priv_data: Private data.
 * @name: Console name.
 * @flags: Open flags.
 * This function will open a console node.
 */
static void *console_open(void *priv_data, const char *name, uint32_t flags)
{
    NODE_PARAM param;
    void *fd = NULL;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(priv_data);

#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data lock. */
    ASSERT(semaphore_obtain(&console_data.lock, MAX_WAIT) != SUCCESS);
#endif /* CONFIG_SEMAPHORE */

    /* Initialize a search parameter. */
    param.name = name;
    param.priv = (void *)fd;

    /* First find a file system to which this call can be forwarded. */
    sll_search(&console_data.list, NULL, &fs_sreach_node, &param, OFFSETOF(CONSOLE, fs.next));

    /* If a node was found. */
    if (param.priv)
    {
        /* Use this FD, we will update it if required. */
        fd = param.priv;
    }

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&console_data.lock);
#endif /* CONFIG_SEMAPHORE */

    if (fd != NULL)
    {
        /* Check if we need to call the underlying function to get a new file
         * descriptor. */
        if (((CONSOLE *)fd)->fs.open != NULL)
        {
            /* Call the underlying API to get the file descriptor. */
            fd = ((CONSOLE *)fd)->fs.open(fd, name, flags);
        }
    }

    /* Return the file descriptor. */
    return (fd);

} /* console_open */

/*
 * console_lock
 * @fd: File descriptor for the console.
 * @timeout: Number of ticks we need to wait for the lock.
 * @return: Success will be returned if lock was successfully acquired.
 * This function will get the lock for a given console.
 */
static int32_t console_lock(void *fd, uint32_t timeout)
{
#ifdef CONFIG_SEMAPHORE
    /* Obtain data lock for this console. */
    return semaphore_obtain(&((CONSOLE *)fd)->lock, timeout);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);
    UNUSED_PARAM(timeout);

    /* Lock scheduler. */
    scheduler_lock();

    /* Return success. */
    return (SUCCESS);
#endif /* CONFIG_SEMAPHORE */
} /* console_lock */

/*
 * console_unlock
 * @fd: File descriptor for the console.
 * This function will release the lock for a given console.
 */
static void console_unlock(void *fd)
{
#ifdef CONFIG_SEMAPHORE
    /* Release data lock for this console. */
    semaphore_release(&((CONSOLE *)fd)->lock);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Enable scheduling. */
    scheduler_unlock();
#endif /* CONFIG_SEMAPHORE */
} /* console_unlock */

#endif /* FS_CONSOLE */
