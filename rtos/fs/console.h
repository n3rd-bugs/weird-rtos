/*
 * console.h
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
#endif /* CONFIG_SEMAPHORE */
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
#endif /* CONFIG_SEMAPHORE */

} CONSOLE_DATA;

/* Function prototypes. */
void console_init(void);
void console_register(CONSOLE *console);
void console_unregister(CONSOLE *console);

#endif /* FS_CONSOLE */
#endif /* CONSOLE_H */
