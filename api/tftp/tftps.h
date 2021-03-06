/*
 * tftps.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _TFTPS_H_
#define _TFTPS_H_
#include <kernel.h>

#ifdef CONFIG_TFTPS
#ifndef CONFIG_NET
#error "Networking stack required for TFTP server."
#endif
#include <net.h>
#ifndef NET_UDP
#error "UDP stack required for TFTP server."
#endif
#include <net_udp.h>

/* TFTP error code definitions. */
#define TFTP_NOT_SUPPORTED      -22000
#define TFTP_LONG_FILENAME      -22001
#define TFTP_ERROR_FS           -22002
#define TFTP_ERROR_EXHAUSTED    -22003
#define TFTP_UNKNOWN_TID        -22004
#define TFTP_OUTOFBOUND_BLOCK   -22005
#define TFTP_FRAME_DROP         -22006

/* TFTP definitions. */
#define TFTP_OP_READ_REQ        (0x1)
#define TFTP_OP_WRITE_REQ       (0x2)
#define TFTP_OP_DATA            (0x3)
#define TFTP_OP_ACK             (0x4)
#define TFTP_OP_ERR             (0x5)

#define TFTP_ERROR_GEN          (0x0)
#define TFTP_ERROR_TID          (0x5)

#define TFTP_ERRMSG_NOT_SUPPORTED   "opcode not supported"
#define TFTP_ERRMSG_FILENAME        "filename too long"
#define TFTP_ERRMSG_FS              "error opening specified file"
#define TFTP_ERRMSG_EXHAUSTED       "connections exhausted"
#define TFTP_ERRMSG_TID             "transaction ID is not known for this request"
#define TFTP_ERRMSG_BLOCK           "an out of bound block we received"

#define TFTP_BUFFER_SIZE        (32)
#define TFTP_BLOCK_SIZE         (512)
#define TFTP_CLI_TIMEOUT        (SOFT_TICKS_PER_SEC)

/* TFTP server structure. */
typedef struct _tftp_server
{
    /* Associated UDP port. */
    UDP_PORT    port;

    /* Condition data for processing requests on for this server. */
    SUSPEND     port_suspend;
    CONDITION   *port_condition;
    FS_PARAM    port_fs_param;

    /* Opened file descriptor. */
    FD          fd;

    /* Client address. */
    SOCKET_ADDRESS  client_address;

    /* Current block number. */
    uint16_t    block_num;

    /* Size of block we just sent. */
    uint16_t    tx_block_len;

    /* Flag to specify that we have just sent the last frame. */
    uint8_t     last_block;

    /* Structure padding. */
    uint8_t     pad[3];

} TFTP_SERVER;

/* Function prototypes. */
void tftp_server_init(TFTP_SERVER *, SOCKET_ADDRESS *, char *);

#endif /* CONFIG_TFTPS */
#endif /* _TFTPS_H_ */
