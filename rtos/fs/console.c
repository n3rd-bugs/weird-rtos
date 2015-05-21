/*
 * console.c
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
#include <fs.h>

#ifdef FS_CONSOLE
#include <string.h>
#include <path.h>
#include <sll.h>
#include <console.h>

/* Console data. */
static CONSOLE_DATA console_data;

/* Function prototypes. */
static void *console_open(char *, uint32_t);
static void console_unlock(void *fd);
static int32_t console_lock(void *fd);

/* File system definition. */
FS console_fs =
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
void console_init()
{
    /* Clear the console data. */
    memset(&console_data, 0, sizeof(CONSOLE_DATA));

#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect global console data. */
    semaphore_create(&console_data.lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

    /* Register console with file system. */
    fs_register(&console_fs);

#ifdef DEBUG_CONSOLE_INIT
    /* Initialize DEBUG console. */
    DEBUG_CONSOLE_INIT();
#endif

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
    OS_ASSERT(semaphore_obtain(&console_data.lock, MAX_WAIT) != SUCCESS);

    /* Create a semaphore to protect this console device. */
    memset(&console->lock, 0, sizeof(SEMAPHORE));
    semaphore_create(&console->lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

    /* This utility is called by drivers for registering consoles for the
     * applicable devices, so no need to check for name conflicts. */
    /* Just push this file system in the list. */
    sll_push(&console_data.list, console, OFFSETOF(CONSOLE, fs.next));

    /* Initialize console FS data. */
    console->fs.get_lock = console_lock;
    console->fs.release_lock = console_unlock;
    console->fs.timeout = MAX_WAIT;

    /* Initialize file system condition. */
    fs_condition_init(&console->fs);

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&console_data.lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif

} /* console_register */

/*
 * console_unregister
 * @console: Console data.
 * This function will unregister a console.
 */
void console_unregister(CONSOLE *console)
{
    /* This could be a file descriptor chain, so destroy it. */
    fs_destroy_chain((FD)&console->fs);

#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Obtain the global data lock. */
    OS_ASSERT(semaphore_obtain(&console_data.lock, MAX_WAIT) != SUCCESS);

    /* Obtain the lock for the console needed to be unregistered. */
    if (semaphore_obtain(&console->lock, MAX_WAIT) == SUCCESS)
    {
#endif
        /* Resume all tasks waiting on this file descriptor. */
        fd_handle_criteria((FD)console, NULL, FS_NODE_DELETED);

#ifdef CONFIG_SEMAPHORE
        /* Delete the console lock. */
        semaphore_destroy(&console->lock);
#endif

        /* Just remove this console from console list. */
        OS_ASSERT(sll_remove(&console_data.list, console, OFFSETOF(CONSOLE, fs.next)) != console);

#ifdef CONFIG_SEMAPHORE
    }

    /* Release the global data lock. */
    semaphore_release(&console_data.lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif

} /* console_unregister */

/*
 * console_open
 * @name: Console name.
 * @flags: Open flags.
 * This function will open a console node.
 */
static void *console_open(char *name, uint32_t flags)
{
    NODE_PARAM param;
    void *fd = NULL;

#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data lock. */
    OS_ASSERT(semaphore_obtain(&console_data.lock, MAX_WAIT) != SUCCESS);
#endif

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
#endif

    if (fd != NULL)
    {
        /* Check if we need to call the underlying function to get a new file
         * descriptor. */
        if (((CONSOLE *)fd)->fs.open != NULL)
        {
            /* Call the underlying API to get the file descriptor. */
            fd = ((CONSOLE *)fd)->fs.open(name, flags);
        }
    }

    /* Return the file descriptor. */
    return (fd);

} /* console_open */

/*
 * console_lock
 * @fd: File descriptor for the console.
 * This function will get the lock for a given console.
 */
static int32_t console_lock(void *fd)
{
#ifdef CONFIG_SEMAPHORE
    /* Obtain data lock for this console. */
    return semaphore_obtain(&((CONSOLE *)fd)->lock, MAX_WAIT);
#else
    /* Remove some compiler warnings. */
    UNUSED_PARAM(fd);

    /* Lock scheduler. */
    scheduler_lock();

    /* Return success. */
    return (SUCCESS);
#endif
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
#endif
} /* console_unlock */

#endif /* FS_CONSOLE */
