/*
 * net_device.h
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
#ifndef _NET_DEVICE_H_
#define _NET_DEVICE_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <fs.h>
#include <semaphore.h>
#include <net.h>
#include <net_condition.h>

/* Network device file descriptor loose definition. */
typedef struct _net_dev NET_DEV;

#ifdef NET_IPV4
#include <net_ipv4.h>
#endif

/* Networking device flags. */
#define NET_DEVICE_UP       0x1

/* Buffer flags. */
#define ETH_FRAME_BCAST     (0x1)

/* Networking device transmit/receive functions. */
typedef int32_t NET_TX (FS_BUFFER_LIST *, uint8_t);
typedef NET_CONDITION_PROCESS NET_RX;

/* Network device file descriptor. */
struct _net_dev
{
#ifdef NET_IPV4
    /* IPv4 device data. */
    IPV4_DEVICE ipv4;
#endif

    /* Networking condition data that will be used to process events on this
     * device. */
    SUSPEND     suspend;
    FS_PARAM    fs_param;

    /* Networking device list member. */
    NET_DEV     *next;

    /* File descriptor linked with this networking device. */
    FD          fd;

    /* Transmit function that will be called to send a packet. */
    NET_TX      *tx;

    /* MTU for this networking device. */
    uint32_t    mtu;

    /* Flags to be maintained for this device. */
    uint32_t    flags;
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

} NET_DEV_DATA;

/* Exported variables. */
extern NET_DEV_DATA net_dev_data;

/* Function prototypes. */
void net_devices_init(void);
void net_register_fd(NET_DEV *, FD, NET_TX *, NET_RX *);
NET_DEV *net_device_get_fd(FD);
void net_device_set_mtu(FD, uint32_t);
uint32_t net_device_get_mtu(FD);
int32_t net_device_buffer_receive(FS_BUFFER_LIST *, uint8_t, uint32_t);
int32_t net_device_buffer_transmit(FS_BUFFER_LIST *, uint8_t, uint8_t);
void net_device_link_up(FD);
void net_device_link_down(FD);

#endif /* CONFIG_NET */
#endif /* _NET_DEVICE_H_ */
