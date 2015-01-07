/*
 * pipe.h
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
#ifndef PIPE_H
#define PIPE_H

#include <os.h>

#ifdef FS_PIPE

/* Message flags. */
#define PIPE_MSG_VALID      0x0001

/* Message data structure. */
typedef struct _msg_data MSG_DATA;
struct _msg_data
{
    /* Message flag. */
    uint32_t    flags;

    /* Message data size. */
    uint32_t    size;
};

/* Pipe file system. */
typedef struct _pipe
{
    /* File system data. */
    FS          fs;

#ifdef CONFIG_SEMAPHORE
    /* Lock for this pipe. */
    SEMAPHORE   lock;
#endif

    /* Pipe message space. */
    char        *data;

    /* Message space size. */
    uint32_t    size;

    /* Internal members. */
    /* Current message. */
    uint32_t    message;

    /* Current free space. */
    uint32_t    free;
} PIPE;

/* Pipe data. */
typedef struct _pipe_data
{
    /* Pipe list. */
    struct _pipe_list
    {
        PIPE    *head;
        PIPE    *tail;
    } list;

#ifdef CONFIG_SEMAPHORE
    /* Data lock. */
    SEMAPHORE   lock;
#endif

} PIPE_DATA;

/* Function prototypes. */
void pipe_init();
void pipe_create(PIPE *pipe);

#endif /* FS_PIPE */

#endif /* PIPE_H */
