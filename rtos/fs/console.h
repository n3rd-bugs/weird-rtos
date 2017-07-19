/*
 * console.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include <kernel.h>

#ifdef FS_CONSOLE

/* Console file system. */
typedef struct _console
{
    /* For now it only has FS data. */
    FS      fs;

#ifdef CONFIG_SEMAPHORE
    /* Data lock. */
    SEMAPHORE   lock;
#endif
} CONSOLE;

/* Console data. */
typedef struct _console_data
{
    /* Console list. */
    struct _con_list
    {
        CONSOLE     *head;
        CONSOLE     *tail;
    } list;

#ifdef CONFIG_SEMAPHORE
    /* Data lock. */
    SEMAPHORE   lock;
#endif

} CONSOLE_DATA;

/* Function prototypes. */
void console_init();
void console_register(CONSOLE *console);
void console_unregister(CONSOLE *console);

#endif /* FS_CONSOLE */
#endif /* CONSOLE_H */
