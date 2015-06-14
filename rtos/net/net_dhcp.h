/*
 * net_dhcp.h
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
#ifndef _NET_DHCP_H_
#define _NET_DHCP_H_
#include <os.h>

#ifdef CONFIG_NET
#include <net.h>

#ifdef NET_DHCP
#define DHCP_CLIENT
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
#define DHCP_OP_REQUEST     (0x01)
#define DHCP_OP_REPLY       (0x02)
#define DHCP_OP_HEADER      (0x00010601)
#define DHCP_MAGIC_COKIE    (0x63825363)

/* DHCP option definitions. */
#define DHCP_OPT_HOST_NAME  (0x0C)
#define DHCP_OPT_REQ_IP     (0x32)
#define DHCP_OPT_LEASE_TIME (0x33)
#define DHCP_OPT_MSG_TYPE   (0x35)
#define DHCP_OPT_SRV_ID     (0x36)
#define DHCP_OPT_END        (0xFF)

/* DHCP message type definitions. */
#define DHCP_MSG_DICOVER    (0x01)
#define DHCP_MSG_OFFER      (0x02)
#define DHCP_MSG_REQUEST    (0x03)
#define DHCP_MSG_ACK        (0x05)
#define DHCP_MSG_NACK       (0x06)

/* Function prototypes. */
int32_t dhcp_add_header(FS_BUFFER *, uint8_t, uint32_t, uint16_t, uint8_t, uint32_t, uint32_t, uint32_t, uint8_t *);
int32_t dhcp_get_header(FS_BUFFER *, uint8_t *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, uint8_t *);
int32_t dhcp_add_option(FS_BUFFER *, uint8_t, void *, uint8_t, uint8_t);
int32_t dhcp_get_option(FS_BUFFER *, uint8_t *, uint8_t *);

#endif /* NET_DHCP */

#endif /* CONFIG_NET */
#endif /* _NET_DHCP_H_ */
