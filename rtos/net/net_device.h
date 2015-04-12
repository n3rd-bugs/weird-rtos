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

/* Net transmit function definition. */
typedef int32_t NET_TX (FS_BUFFER *);

/* Network device file descriptor. */
typedef struct _net_dev NET_DEV;
struct _net_dev
{
    /* Networking device list member. */
    NET_DEV     *next;

    /* File descriptor linked with this networking device. */
    FD          fd;

    /* File system watchers. */
    FS_DATA_WATCHER         data_watcher;
    FS_CONNECTION_WATCHER   connection_watcher;

#ifdef CONFIG_SEMAPHORE
    /* Protection for this networking device. */
    SEMAPHORE   lock;
#endif

    /* Transmit function that will be called to send a packet. */
    NET_TX      *tx;

#ifdef NET_IPV4
    /* IP address assigned to this interface. */
    uint32_t    ipv4_address;
#endif

};

/* Network device global data. */
typedef struct _net_dev_data
{
    /* Networking devices list. */
    struct _net_device_list
    {
        NET_DEV     *head;
        NET_DEV     *tail;
    } devices;

#ifdef CONFIG_SEMAPHORE
    /* Protection for global data. */
    SEMAPHORE   lock;
#endif

} NET_DEV_DATA;

/* Function prototypes. */
void net_devices_init();
void net_register_fd(NET_DEV *, FD, NET_TX *);
NET_DEV *net_device_get_fd(FD);
void net_device_buffer_receive(FS_BUFFER *, uint8_t);
int32_t net_device_buffer_transmit(FS_BUFFER *, uint8_t);
void net_device_connected(void *, void *);
void net_device_disconnected(void *, void *);
void net_device_rx_watcher(void *, void *);

#endif /* CONFIG_NET */
#endif /* _NET_DEVICE_H_ */
