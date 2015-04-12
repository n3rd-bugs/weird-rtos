/*
 * net_buffer.h
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
#ifndef _NET_BUFFER_H_
#define _NET_BUFFER_H_
#include <os.h>

#ifdef CONFIG_NET
#include <fs.h>
#include <semaphore.h>

/* Net buffer ID used to tell the networking stack that a buffer belongs to it. */
#define NET_BUFFER_ID               0x8A6EF4C6

/* Protocol definitions that the networking stack will be expecting on a buffer
 * to invoke an interrupt routine for that. */
#define NET_PROTO_IPV4              0x01

/* Net buffer configuration. */
#define NET_BUFFER_RX_STACK_SIZE    1024

/* Global network buffer data. */
typedef struct _net_buffer_fs
{
    /* Contains the file system hooks for networking buffers. */
    FS      fs;

    /* Net buffer list, this will contain the buffers we still need to
     * process. */
    struct _net_buffer_fs_buffer_list
    {
        FS_BUFFER       *head;
        FS_BUFFER       *tail;
    } buffer_list;

#ifdef CONFIG_SEMAPHORE
    /* Data lock to protect network buffers. */
    SEMAPHORE   lock;
#endif

} NET_BUFFER_FS;

/* Exported variables. */
extern FD net_buff_fd;

/* Function prototypes. */
void net_buffer_init();

#endif /* CONFIG_NET */
#endif /* _NET_BUFFER_H_ */
