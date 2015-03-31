/*
 * net_device.h
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
#ifndef _NET_DEVICE_H_
#define _NET_DEVICE_H_
#include <os.h>

#ifdef CONFIG_NET
#include <fs.h>
#include <semaphore.h>
#include <net.h>

/* Network device file descriptor. */
typedef struct _net_dev
{
    /* File system watchers. */
    FS_DATA_WATCHER         data_watcher;
    FS_CONNECTION_WATCHER   connection_watcher;

#ifdef CONFIG_SEMAPHORE
    /* Protection for this networking device. */
    SEMAPHORE   lock;
#endif
} NET_DEV;

/* Function prototypes. */
void net_register_fd(NET_DEV *, FD);
void net_device_buffer_receive(FS_BUFFER *, uint8_t);
void net_device_connected(void *, void *);
void net_device_disconnected(void *, void *);
void net_device_rx_watcher(void *, void *);

#endif /* CONFIG_NET */
#endif /* _NET_DEVICE_H_ */
