/*
 * pipe.h
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
#ifndef PIPE_H
#define PIPE_H

#include <kernel.h>

#ifdef FS_PIPE

/* Message flags. */
#define PIPE_MSG_VALID      0x1

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
    uint8_t     *data;

    /* Message space size. */
    uint32_t    size;

    /* Internal members. */
    /* Current message. */
    uint32_t    message;

    /* Current free space. */
    uint32_t    free;
} PIPE;

/* Pipe data, used to maintain global PIPE data. */
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
void pipe_init(void);
void pipe_create(PIPE *pipe, char *, uint8_t *, uint32_t);
void pipe_destroy(PIPE *);

/* If user wants to use a pipe as a circular buffer. */
int32_t pipe_write(void *, const uint8_t *, int32_t);
int32_t pipe_read(void *, uint8_t *, int32_t);

#endif /* FS_PIPE */
#endif /* PIPE_H */
