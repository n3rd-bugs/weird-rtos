/*
 * weird_view_server.h
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
#ifndef _WEIRD_VIEW_SERVER_H_
#define _WEIRD_VIEW_SERVER_H_
#include <kernel.h>

#ifdef CONFIG_WEIRD_VIEW
#ifndef CONFIG_NET
#error "Networking stack required for weird view."
#endif
#include <net.h>
#ifndef NET_UDP
#error "UDP stack required for weird view."
#endif
#include <net_udp.h>
#include <weird_view.h>

/* Weird view log plugin function prototype. */
typedef int32_t WV_GET_LOG_DATA (uint16_t, FS_BUFFER_LIST *);

/* Weird view switch plugin function prototype. */
typedef int32_t WV_GET_SWITCH_DATA (uint16_t, uint8_t *);
typedef void WV_POC_SWITCH_REQ (uint16_t, uint8_t);

/* Weird view analog plugin function prototype. */
typedef int32_t WV_GET_ANALOG_DATA (uint16_t, uint32_t *, uint32_t *, uint32_t *);

/* Weird view plugin structure. */
typedef struct _weird_view_plugin
{
    /* Name of this plugin. */
    P_STR_T     name;

    /* Plugin data pointer. */
    void        *data;

    /* Plugin request pointer. */
    void        *request;

    /* Unique plugin ID. */
    uint16_t    id;

    /* Type of this plugin. */
    uint8_t     type;

    /* Structure padding. */
    uint8_t     pad[1];

} WEIRD_VIEW_PLUGIN;

/* Weird view server structure. */
typedef struct _weird_view_server
{
    /* Associated UDP port. */
    UDP_PORT            port;

    /* Condition data for processing requests on for this server. */
    SUSPEND             port_suspend;
    CONDITION           *port_condition;
    FS_PARAM            port_fs_param;

    /* Device name to be used. */
    char                *device_name;

    /* Plugin database. */
    WEIRD_VIEW_PLUGIN   *plugin;
    uint32_t            num_plugin;

} WEIRD_VIEW_SERVER;

void weird_view_server_init(WEIRD_VIEW_SERVER *, SOCKET_ADDRESS *, char *, WEIRD_VIEW_PLUGIN *, uint32_t);

#endif /* CONFIG_WEIRD_VIEW */
#endif /* _WEIRD_VIEW_SERVER_H_ */
