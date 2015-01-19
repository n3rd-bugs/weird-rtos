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
#include <string.h>
#include <serial.h>
#include <path.h>
#include <sll.h>

#ifdef FS_CONSOLE

/* Console data. */
CONSOLE_DATA console_data;

/* Function prototypes. */
void *console_open(char *, uint32_t);

/* File system definition. */
FS console_fs;

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

    /* Clear the console fs. */
    memset(&console_fs, 0, sizeof(FS));

    /* Initialize console data. */
    console_fs.name = "\\console";
    console_fs.open = console_open;

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
    semaphore_obtain(&console_data.lock, MAX_WAIT);
#endif

    /* Just push this file system in the list. */
    sll_push(&console_data.list, console, OFFSETOF(CONSOLE, fs.next));

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&console_data.lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif
}

/*
 * console_open
 * @name: Console name.
 * @flags: Open flags.
 * This function will open a console node.
 */
void *console_open(char *name, uint32_t flags)
{
    NODE_PARAM param;
    void *fd = NULL;

#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data lock. */
    semaphore_obtain(&console_data.lock, MAX_WAIT);
#endif

    /* Initialize a search parameter. */
    param.name = name;
    param.priv = (void *)fd;

    /* First find a file system to which this call can be forwarded. */
    sll_search(&console_data.list, NULL, fs_sreach_node, &param, OFFSETOF(CONSOLE, fs.next));

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

#endif /* FS_CONSOLE */
