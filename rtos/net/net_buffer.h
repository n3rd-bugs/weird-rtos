/*
 * net_buffer.h
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
#ifndef _NET_BUFFER_H_
#define _NET_BUFFER_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <fs.h>
#include <semaphore.h>
#include <net_condition.h>

/* Protocol definitions that the networking stack will be expecting on a buffer
 * to invoke an interrupt routine for that. */
#define NET_PROTO_IPV4              0x01
#define NET_PROTO_ARP               0x02

/* Global network buffer data. */
typedef struct _net_buffer_fs
{
    /* Contains the file system hooks for networking buffers. */
    FS      fs;

    /* Networking buffer list, this will contain the buffers we still need to
     * process. */
    struct _net_buffer_list
    {
        FS_BUFFER_LIST  *head;
        FS_BUFFER_LIST  *tail;
    } buffer_list;

#ifdef CONFIG_SEMAPHORE
    /* Data lock to protect network buffers. */
    SEMAPHORE   lock;
#endif

    /* File system suspend parameter to be used to wait on this file descriptor. */
    FS_PARAM    fs_param;

} NET_BUFFER_FS;

/* Exported variables. */
extern FD net_buff_fd;

/* Function prototypes. */
void net_buffer_init(void);
void net_buffer_get_condition(CONDITION **, SUSPEND *, NET_CONDITION_PROCESS **);

#endif /* CONFIG_NET */
#endif /* _NET_BUFFER_H_ */
