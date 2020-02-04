/*
 * net_dhcp.h
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
#ifndef _NET_DHCP_H_
#define _NET_DHCP_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_DHCP
#include <net_dhcp_config.h>
#ifndef CONFIG_ETHERNET
#error "For now only devices using ethernet are supported."
#endif

/* DHCP UDP port configurations. */
#define DHCP_SRV_PORT       (67)
#define DHCP_CLI_PORT       (68)

/* DHCP header definitions. */
#define DHCP_HDR_OP_SHIFT       (24)
#define DHCP_HDR_BCAST_SHIFT    (15)
#define DHCP_HDR_CHADDR_LEN     (16)

/* DHCP operation definition. */
/* By default we will be using ethernet type/address length and hop limit will
 * be 1. */
#define DHCP_OP_REQUEST     (0x1)
#define DHCP_OP_REPLY       (0x2)
#define DHCP_OP_HEADER      (0x10601)
#define DHCP_MAGIC_COKIE    (0x63825363)

/* DHCP option definitions. */
#define DHCP_OPT_NETWORK    (0x1)
#define DHCP_OPT_GATEWAY    (0x3)
#define DHCP_OPT_HOST_NAME  (0xC)
#define DHCP_OPT_REQ_IP     (0x32)
#define DHCP_OPT_LEASE_TIME (0x33)
#define DHCP_OPT_MSG_TYPE   (0x35)
#define DHCP_OPT_SRV_ID     (0x36)
#define DHCP_OPT_END        (0xFF)

/* DHCP message type definitions. */
#define DHCP_MSG_DICOVER    (0x1)
#define DHCP_MSG_OFFER      (0x2)
#define DHCP_MSG_REQUEST    (0x3)
#define DHCP_MSG_ACK        (0x5)
#define DHCP_MSG_NACK       (0x6)

/* Function prototypes. */
int32_t dhcp_add_header(FS_BUFFER_LIST *, uint8_t, uint32_t, uint16_t, uint8_t, uint32_t, uint32_t, uint32_t, uint8_t *);
int32_t dhcp_get_header(FS_BUFFER_LIST *, uint8_t *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, uint8_t *);
int32_t dhcp_add_option(FS_BUFFER_LIST *, uint8_t, uint8_t, void *, uint8_t);
int32_t dhcp_get_option(FS_BUFFER_LIST *, uint8_t *, uint8_t *);

#endif /* NET_DHCP */

#endif /* CONFIG_NET */
#endif /* _NET_DHCP_H_ */
