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
 * (in any form) the author will not be liable for any outcome from it's direct
 * or indirect use.
 */
#ifndef _NET_DEVICE_H_
#define _NET_DEVICE_H_
#include <os.h>

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
#define NET_DEVICE_UP       0x01

/* Buffer flags. */
#define ETH_FRAME_BCAST     (0x01)

/* Networking device transmit/receive functions. */
typedef int32_t NET_TX (FS_BUFFER *, uint8_t);
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
void net_devices_init();
void net_register_fd(NET_DEV *, FD, NET_TX *, NET_RX *);
NET_DEV *net_device_get_fd(FD);
void net_device_set_mtu(FD, uint32_t);
uint32_t net_device_get_mtu(FD);
int32_t net_device_buffer_receive(FS_BUFFER *, uint8_t, uint32_t);
int32_t net_device_buffer_transmit(FS_BUFFER *, uint8_t, uint8_t);
void net_device_link_up(FD);
void net_device_link_down(FD);

#endif /* CONFIG_NET */
#endif /* _NET_DEVICE_H_ */
